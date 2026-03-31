#include "RecipeManager.h"
#include "Oxford133.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

RecipeManager::RecipeManager(Oxford133 *device, QObject *parent)
    : QObject(parent)
    , m_device(device)
    , m_currentCycle(0)
    , m_currentStepIndex(0)
    , m_running(false)
{
}

RecipeManager::~RecipeManager()
{
}

void RecipeManager::addRecipe(const Recipe &recipe)
{
    m_recipes.append(recipe);
}

void RecipeManager::removeRecipe(const QString &name)
{
    for (int i = 0; i < m_recipes.size(); ++i) {
        if (m_recipes[i].name == name) {
            m_recipes.removeAt(i);
            return;
        }
    }
}

RecipeManager::Recipe RecipeManager::getRecipe(const QString &name) const
{
    for (const Recipe &r : m_recipes) {
        if (r.name == name) {
            return r;
        }
    }
    return Recipe();
}

QStringList RecipeManager::getRecipeNames() const
{
    QStringList names;
    for (const Recipe &r : m_recipes) {
        names.append(r.name);
    }
    return names;
}

bool RecipeManager::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(QStringLiteral("无法打开文件: %1").arg(filePath));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        emit errorOccurred(QStringLiteral("JSON解析错误: %1").arg(err.errorString()));
        return false;
    }

    m_recipes.clear();
    QJsonArray jsonRecipes = doc.array();
    for (const QJsonValue &val : jsonRecipes) {
        QJsonObject obj = val.toObject();
        Recipe recipe;
        recipe.name = obj["name"].toString();
        recipe.cycles = obj["cycles"].toInt();

        QJsonArray jsonSteps = obj["steps"].toArray();
        for (const QJsonValue &stepVal : jsonSteps) {
            QJsonObject stepObj = stepVal.toObject();
            RecipeStep step;
            step.stepNumber = stepObj["step"].toInt();
            step.rfPower = stepObj["rfPower"].toDouble();
            step.icpPower = stepObj["icpPower"].toDouble();
            step.pressure = stepObj["pressure"].toDouble();
            step.gasFlow1 = stepObj["gasFlow1"].toDouble();
            step.gasFlow2 = stepObj["gasFlow2"].toDouble();
            step.gasFlow3 = stepObj["gasFlow3"].toDouble();
            step.gasFlow4 = stepObj["gasFlow4"].toDouble();
            step.temperature = stepObj["temperature"].toDouble();
            step.time = stepObj["time"].toInt();
            recipe.steps.append(step);
        }
        m_recipes.append(recipe);
    }

    return true;
}

bool RecipeManager::saveToFile(const QString &filePath, const QString &recipeName)
{
    Recipe recipe = getRecipe(recipeName);
    if (recipe.name.isEmpty()) {
        emit errorOccurred(QStringLiteral("配方不存在: %1").arg(recipeName));
        return false;
    }

    QJsonArray jsonSteps;
    for (const RecipeStep &step : recipe.steps) {
        QJsonObject stepObj;
        stepObj["step"] = step.stepNumber;
        stepObj["rfPower"] = step.rfPower;
        stepObj["icpPower"] = step.icpPower;
        stepObj["pressure"] = step.pressure;
        stepObj["gasFlow1"] = step.gasFlow1;
        stepObj["gasFlow2"] = step.gasFlow2;
        stepObj["gasFlow3"] = step.gasFlow3;
        stepObj["gasFlow4"] = step.gasFlow4;
        stepObj["temperature"] = step.temperature;
        stepObj["time"] = step.time;
        jsonSteps.append(stepObj);
    }

    QJsonObject recipeObj;
    recipeObj["name"] = recipe.name;
    recipeObj["cycles"] = recipe.cycles;
    recipeObj["steps"] = jsonSteps;

    QJsonArray jsonRecipes;
    jsonRecipes.append(recipeObj);

    QJsonDocument doc(jsonRecipes);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred(QStringLiteral("无法写入文件: %1").arg(filePath));
        return false;
    }

    file.write(doc.toJson());
    file.close();
    return true;
}

void RecipeManager::setCurrentRecipe(const QString &name)
{
    m_currentRecipeName = name;
    m_currentCycle = 0;
    m_currentStepIndex = 0;
}

bool RecipeManager::executeNextStep()
{
    if (m_currentRecipeName.isEmpty()) {
        emit errorOccurred(QStringLiteral("未选择配方"));
        return false;
    }

    Recipe recipe = getRecipe(m_currentRecipeName);
    if (recipe.name.isEmpty()) {
        emit errorOccurred(QStringLiteral("配方不存在"));
        return false;
    }

    if (m_currentStepIndex >= recipe.steps.size()) {
        // 所有步骤完成
        if (m_currentCycle + 1 < recipe.cycles) {
            // 进入下一个循环
            m_currentCycle++;
            m_currentStepIndex = 0;
        } else {
            // 配方全部完成
            emit recipeCompleted(m_currentRecipeName);
            m_running = false;
            return true;
        }
    }

    return executeCurrentStep();
}

bool RecipeManager::executeCurrentStep()
{
    Recipe recipe = getRecipe(m_currentRecipeName);
    if (recipe.name.isEmpty() || m_currentStepIndex >= recipe.steps.size()) {
        return false;
    }

    RecipeStep step = recipe.steps[m_currentStepIndex];

    // 设置工艺参数
    if (step.rfPower >= 0) m_device->setRFPower(step.rfPower);
    if (step.icpPower >= 0) m_device->setICPPower(step.icpPower);
    if (step.pressure >= 0) m_device->setPressure(step.pressure);
    if (step.temperature > -40) m_device->setTemperature(step.temperature);

    // 设置气体流量
    if (step.gasFlow1 >= 0) m_device->setGasFlow(1, step.gasFlow1);
    if (step.gasFlow2 >= 0) m_device->setGasFlow(2, step.gasFlow2);
    if (step.gasFlow3 >= 0) m_device->setGasFlow(3, step.gasFlow3);
    if (step.gasFlow4 >= 0) m_device->setGasFlow(4, step.gasFlow4);

    emit recipeStepStarted(m_currentStepIndex + 1, step);
    emit progressUpdated(m_currentStepIndex + 1, recipe.steps.size(), m_currentCycle + 1, recipe.cycles);

    // 启动工艺
    m_device->startProcess();
    m_running = true;

    return true;
}

bool RecipeManager::runFullRecipe()
{
    if (m_currentRecipeName.isEmpty()) {
        emit errorOccurred(QStringLiteral("未选择配方"));
        return false;
    }

    Recipe recipe = getRecipe(m_currentRecipeName);
    if (recipe.name.isEmpty()) {
        emit errorOccurred(QStringLiteral("配方不存在"));
        return false;
    }

    m_currentCycle = 0;
    m_currentStepIndex = 0;
    m_running = true;

    emit recipeStarted(m_currentRecipeName);

    return executeCurrentStep();
}

void RecipeManager::stopRecipe()
{
    m_device->stopProcess();
    m_running = false;
    emit recipeStopped(m_currentRecipeName);
}

void RecipeManager::createDefaultRecipes()
{
    // 默认配方: Si 刻蚀
    Recipe siEtch;
    siEtch.name = QStringLiteral("Si 刻蚀");
    siEtch.cycles = 1;

    RecipeStep step1;
    step1.stepNumber = 1;
    step1.rfPower = 100;
    step1.icpPower = 1000;
    step1.pressure = 10;
    step1.gasFlow1 = 50;   // SF6
    step1.gasFlow2 = 10;   // Ar
    step1.gasFlow3 = 0;
    step1.gasFlow4 = 0;
    step1.temperature = 20;
    step1.time = 60;
    siEtch.steps.append(step1);

    m_recipes.append(siEtch);

    // 默认配方: SiO2 刻蚀
    Recipe sio2Etch;
    sio2Etch.name = QStringLiteral("SiO2 刻蚀");
    sio2Etch.cycles = 1;

    RecipeStep step2;
    step2.stepNumber = 1;
    step2.rfPower = 150;
    step2.icpPower = 1500;
    step2.pressure = 15;
    step2.gasFlow1 = 30;   // CF4
    step2.gasFlow2 = 20;  // Ar
    step2.gasFlow3 = 0;
    step2.gasFlow4 = 0;
    step2.temperature = 20;
    step2.time = 120;
    sio2Etch.steps.append(step2);

    m_recipes.append(sio2Etch);
}
