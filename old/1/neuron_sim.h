#ifndef NEURON_SIM_H
#define NEURON_SIM_H

#include <vector>
#include <cmath>

// 向量类，用于表示位置和方向
struct Vector2D {
    double x, y;
    
    Vector2D(double x = 0, double y = 0) : x(x), y(y) {}
    
    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator*(double scalar) const;
    void normalize();
};

// 计算两点之间的距离
double distance(const Vector2D& a, const Vector2D& b);

// 突触类，表示神经元之间的连接
struct Synapse {
    int targetNeuron;  // 目标神经元的索引
    double strength;   // 连接强度
    double lastUsed;   // 最后使用时间
    bool isActive;     // 突触是否活跃
    
    // 突触可塑性参数
    static constexpr double STRENGTH_MIN = 0.1;
    static constexpr double STRENGTH_MAX = 1.0;
    static constexpr double LEARNING_RATE = 0.05;
    static constexpr double DECAY_RATE = 0.01;
    static constexpr double INACTIVITY_THRESHOLD = 50;  // 步数
    
    Synapse(int target, double str, double initTime);
    
    void strengthen();
    void decay();
    void checkInactivity(double currentTime);
    void use(double currentTime);
};

// 神经元类
class Neuron {
private:
    Vector2D position;    // 位置
    Vector2D direction;   // 移动方向
    double speed;         // 移动速度
    std::vector<Synapse> synapses;  // 突触连接
    double activationLevel;  // 激活水平
    double potential;      // 膜电位
    bool isFiring;         // 是否正在发放脉冲
    
    // 神经元电生理参数
    static constexpr double RESTING_POTENTIAL = -70.0;
    static constexpr double THRESHOLD_POTENTIAL = -55.0;
    static constexpr double REFRACTORY_PERIOD = 5.0;
    double lastFired;      // 上次发放时间
    double activationDecay; // 激活衰减率
    
public:
    Neuron(double x, double y);
    
    const Vector2D& getPosition() const;
    
    void move(double width, double height);
    
    void connectTo(int targetNeuron, double strength, double currentTime);
    
    bool isCloseEnough(const Neuron& other, double threshold) const;
    
    std::vector<Synapse> getActiveSynapses() const;
    
    void receiveSignal(double signalStrength, double currentTime);
    
    void fire(double currentTime);
    
    void update(double currentTime);
    
    void updateSynapses(double currentTime);
    
    bool firing() const;
    double getActivationLevel() const;
};

// 神经网络模拟类
class NeuralNetworkSimulation {
public:
    std::vector<Neuron> neurons;
    double width, height;
    double connectionThreshold;
    int currentStep;
    
    NeuralNetworkSimulation(int numNeurons, double w, double h, double threshold);
    
    void step();
};

#endif // NEURON_SIM_H

