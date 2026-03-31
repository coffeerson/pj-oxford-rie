#ifndef RECIPEMANAGER_H
#define RECIPEMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariantMap>

/**
 * RecipeManager - 工艺配方管理类
 * 支持最多 25 步, 99 循环的工艺配方
 */
class Oxford133;

class RecipeManager : public QObject
{
    Q_OBJECT

public:
    // 单个工艺步骤
    struct RecipeStep {
        int stepNumber;
        double rfPower;       // RF 功率 (W)
        double icpPower;      // ICP 功率 (W)
        double pressure;      // 压力 (mbar)
        double gasFlow1;      // 气体1流量 (seem)
        double gasFlow2;      // 气体2流量
        double gasFlow3;      // 气体3流量
        double gasFlow4;      // 气体4流量
        double temperature;   // 温度 (°C)
        int time;            // 时间 (秒)
    };

    // 完整工艺配方
    struct Recipe {
        QString name;
        int cycles;           // 循环次数 1-99
        QVector<RecipeStep> steps;  // 步骤列表，最多 25 步
    };

    explicit RecipeManager(Oxford133 *device, QObject *parent = nullptr);
    ~RecipeManager();

    // 配方管理
    void addRecipe(const Recipe &recipe);
    void removeRecipe(const QString &name);
    Recipe getRecipe(const QString &name) const;
    QStringList getRecipeNames() const;
    bool loadFromFile(const QString &filePath);
    bool saveToFile(const QString &filePath, const QString &recipeName);

    // 当前配方执行
    void setCurrentRecipe(const QString &name);
    QString getCurrentRecipeName() const { return m_currentRecipeName; }
    bool executeNextStep();
    bool executeCurrentStep();
    bool runFullRecipe();        // 执行完整配方
    void stopRecipe();           // 停止配方执行

    // 配方模板
    void createDefaultRecipes();

signals:
    void recipeStarted(const QString &recipeName);
    void recipeStepStarted(int stepNumber, const RecipeStep &step);
    void recipeStepCompleted(int stepNumber);
    void recipeCompleted(const QString &recipeName);
    void recipeStopped(const QString &recipeName);
    void errorOccurred(const QString &error);
    void progressUpdated(int currentStep, int totalSteps, int currentCycle, int totalCycles);

private:
    Oxford133 *m_device;
    QVector<Recipe> m_recipes;
    QString m_currentRecipeName;
    int m_currentCycle;
    int m_currentStepIndex;
    bool m_running;
};

#endif // RECIPEMANAGER_H
