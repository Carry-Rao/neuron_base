#include "neuron_sim.h"
#include <random>
#include <chrono>
#include <algorithm>

// Vector2D 实现
Vector2D Vector2D::operator+(const Vector2D& other) const {
    return Vector2D(x + other.x, y + other.y);
}

Vector2D Vector2D::operator*(double scalar) const {
    return Vector2D(x * scalar, y * scalar);
}

void Vector2D::normalize() {
    double len = sqrt(x*x + y*y);
    if (len > 0) {
        x /= len;
        y /= len;
    }
}

// 距离计算
double distance(const Vector2D& a, const Vector2D& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

// Synapse 实现
Synapse::Synapse(int target, double str, double initTime) 
    : targetNeuron(target), strength(str), lastUsed(initTime), isActive(true) {}

void Synapse::strengthen() {
    strength = std::min(STRENGTH_MAX, strength + LEARNING_RATE);
}

void Synapse::decay() {
    strength = std::max(STRENGTH_MIN, strength - DECAY_RATE);
}

void Synapse::checkInactivity(double currentTime) {
    if (currentTime - lastUsed > INACTIVITY_THRESHOLD) {
        isActive = false;
    }
}

void Synapse::use(double currentTime) {
    lastUsed = currentTime;
    if (!isActive) {
        isActive = true;
    }
}

// Neuron 实现
Neuron::Neuron(double x, double y) : position(x, y), 
                                 activationLevel(0.0),
                                 potential(RESTING_POTENTIAL),
                                 isFiring(false),
                                 lastFired(-REFRACTORY_PERIOD),
                                 activationDecay(0.95) {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> dirDist(-1.0, 1.0);
    std::uniform_real_distribution<double> speedDist(0.1, 0.5);
    
    direction = Vector2D(dirDist(gen), dirDist(gen));
    direction.normalize();
    speed = speedDist(gen);
}

const Vector2D& Neuron::getPosition() const { return position; }

void Neuron::move(double width, double height) {
    position = position + direction * speed;
    
    // 边界检查，碰到边界反弹
    if (position.x < 0 || position.x > width) {
        direction.x *= -1;
        position.x = std::max(0.0, std::min(width, position.x));
    }
    
    if (position.y < 0 || position.y > height) {
        direction.y *= -1;
        position.y = std::max(0.0, std::min(height, position.y));
    }
    
    // 微小的随机方向变化
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::normal_distribution<double> dirChange(0.0, 0.1);
    
    direction.x += dirChange(gen);
    direction.y += dirChange(gen);
    direction.normalize();
}

void Neuron::connectTo(int targetNeuron, double strength, double currentTime) {
    for (const auto& synapse : synapses) {
        if (synapse.targetNeuron == targetNeuron && synapse.isActive) {
            return;
        }
    }
    synapses.emplace_back(targetNeuron, strength, currentTime);
}

bool Neuron::isCloseEnough(const Neuron& other, double threshold) const {
    return distance(position, other.position) < threshold;
}

std::vector<Synapse> Neuron::getActiveSynapses() const {
    std::vector<Synapse> result;
    for (const auto& synapse : synapses) {
        if (synapse.isActive) {
            result.push_back(synapse);
        }
    }
    return result;
}

void Neuron::receiveSignal(double signalStrength, double currentTime) {
    if (currentTime - lastFired < REFRACTORY_PERIOD) {
        return;
    }
    
    potential += signalStrength * 10.0;
    
    if (potential >= THRESHOLD_POTENTIAL) {
        fire(currentTime);
    }
}

void Neuron::fire(double currentTime) {
    isFiring = true;
    lastFired = currentTime;
    activationLevel = 1.0;
    potential = RESTING_POTENTIAL;
}

void Neuron::update(double currentTime) {
    activationLevel *= activationDecay;
    
    if (!isFiring && currentTime - lastFired >= REFRACTORY_PERIOD) {
        if (potential > RESTING_POTENTIAL) {
            potential -= 1.0;
        }
    }
    
    isFiring = false;
    updateSynapses(currentTime);
}

void Neuron::updateSynapses(double currentTime) {
    for (auto& synapse : synapses) {
        if (synapse.isActive) {
            synapse.decay();
            
            if (currentTime - lastFired < 1.0) {
                synapse.strengthen();
            }
            
            synapse.checkInactivity(currentTime);
        }
    }
}

bool Neuron::firing() const { return isFiring; }
double Neuron::getActivationLevel() const { return activationLevel; }

// NeuralNetworkSimulation 实现
NeuralNetworkSimulation::NeuralNetworkSimulation(int numNeurons, double w, double h, double threshold)
    : width(w), height(h), connectionThreshold(threshold), currentStep(0) {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> xDist(0, width);
    std::uniform_real_distribution<double> yDist(0, height);
    
    for (int i = 0; i < numNeurons; ++i) {
        neurons.emplace_back(xDist(gen), yDist(gen));
    }
}

void NeuralNetworkSimulation::step() {
    currentStep++;
    
    // 移动所有神经元
    for (auto& neuron : neurons) {
        neuron.move(width, height);
    }
    
    // 检查并建立新的连接
    for (size_t i = 0; i < neurons.size(); ++i) {
        for (size_t j = 0; j < neurons.size(); ++j) {
            if (i != j && neurons[i].isCloseEnough(neurons[j], connectionThreshold)) {
                double dist = distance(neurons[i].getPosition(), neurons[j].getPosition());
                double strength = 0.5 + (0.5 * (1.0 - (dist / connectionThreshold)));
                neurons[i].connectTo(j, strength, currentStep);
            }
        }
    }
    
    // 处理神经元激活和信号传递
    std::vector<int> firingNeurons;
    for (size_t i = 0; i < neurons.size(); ++i) {
        if (neurons[i].firing()) {
            firingNeurons.push_back(i);
        }
    }
    
    for (int source : firingNeurons) {
        const auto& synapses = neurons[source].getActiveSynapses();
        for (const auto& synapse : synapses) {
            int target = synapse.targetNeuron;
            double signal = synapse.strength * neurons[source].getActivationLevel();
            neurons[target].receiveSignal(signal, currentStep);
        }
    }
    
    // 更新所有神经元状态
    for (auto& neuron : neurons) {
        neuron.update(currentStep);
    }
    
    // 随机激活一些神经元
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> activationProb(0.0, 1.0);
    const double ACTIVATION_CHANCE = 0.05;
    
    for (auto& neuron : neurons) {
        if (activationProb(gen) < ACTIVATION_CHANCE) {
            neuron.fire(currentStep);
        }
    }
}

