#include "neuron_sim.h"
#include "visualization.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// 加载PNG图片（同训练代码）
std::vector<std::vector<double>> load_png_image(const std::string& path, int target_width, int target_height) {
    std::vector<std::vector<double>> img;
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 1);
    if (!data) {
        std::cerr << "无法加载图片: " << path << " (" << stbi_failure_reason() << ")" << std::endl;
        return img;
    }
    
    img.resize(target_height, std::vector<double>(target_width, 0.0));
    double x_ratio = static_cast<double>(width) / target_width;
    double y_ratio = static_cast<double>(height) / target_height;
    
    for (int y = 0; y < target_height; ++y) {
        for (int x = 0; x < target_width; ++x) {
            int src_x = static_cast<int>(x * x_ratio);
            int src_y = static_cast<int>(y * y_ratio);
            int idx = src_y * width + src_x;
            img[y][x] = 1.0 - (static_cast<double>(data[idx]) / 255.0);
        }
    }
    
    stbi_image_free(data);
    return img;
}

// 加载训练结果
bool load_training_result(NeuralNetworkSimulation& sim, const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开训练结果文件: " << path << std::endl;
        return false;
    }
    
    // 读取神经元数量
    size_t num_neurons;
    file.read(reinterpret_cast<char*>(&num_neurons), sizeof(num_neurons));
    
    // 重新初始化神经元网络
    sim = NeuralNetworkSimulation(num_neurons, sim.width, sim.height, sim.connectionThreshold);
    
    // 读取每个神经元的连接信息
    for (size_t i = 0; i < num_neurons; ++i) {
        size_t num_synapses;
        file.read(reinterpret_cast<char*>(&num_synapses), sizeof(num_synapses));
        
        for (size_t j = 0; j < num_synapses; ++j) {
            int target;
            double strength, last_used;
            file.read(reinterpret_cast<char*>(&target), sizeof(target));
            file.read(reinterpret_cast<char*>(&strength), sizeof(strength));
            file.read(reinterpret_cast<char*>(&last_used), sizeof(last_used));
            
            sim.neurons[i].connectTo(target, strength, last_used);
        }
    }
    
    return true;
}

// 识别图片中的数字
int recognize_digit(NeuralNetworkSimulation& sim, const std::vector<std::vector<double>>& img) {
    const int INPUT_WIDTH = img[0].size();
    const int INPUT_HEIGHT = img.size();
    const int NUM_DIGITS = 10;
    
    // 激活输入层神经元
    for (int y = 0; y < INPUT_HEIGHT; ++y) {
        for (int x = 0; x < INPUT_WIDTH; ++x) {
            int neuron_idx = y * INPUT_WIDTH + x;
            if (img[y][x] > 0.5 && neuron_idx < sim.neurons.size()) {
                sim.neurons[neuron_idx].fire(sim.currentStep);
            }
        }
    }
    
    // 运行识别步骤
    for (int step = 0; step < 100; ++step) {
        sim.step();
    }
    
    // 检查输出层神经元激活情况（最后10个神经元）
    std::vector<double> activation_levels(NUM_DIGITS, 0.0);
    int output_start = sim.neurons.size() - NUM_DIGITS;
    
    for (int i = 0; i < NUM_DIGITS; ++i) {
        if (output_start + i < sim.neurons.size()) {
            activation_levels[i] = sim.neurons[output_start + i].getActivationLevel();
        }
    }
    
    // 返回激活水平最高的数字
    return std::distance(activation_levels.begin(), 
                        std::max_element(activation_levels.begin(), activation_levels.end()));
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <待识别图片路径>" << std::endl;
        return 1;
    }
    
    const int SIM_WIDTH = 1000;
    const int SIM_HEIGHT = 800;
    const int INPUT_WIDTH = 28;
    const int INPUT_HEIGHT = 28;
    const int NUM_NEURONS = INPUT_WIDTH * INPUT_HEIGHT + 10;
    const double THRESHOLD = 150;
    
    // 创建神经网络模拟
    NeuralNetworkSimulation simulation(NUM_NEURONS, SIM_WIDTH, SIM_HEIGHT, THRESHOLD);
    
    // 加载训练结果
    if (!load_training_result(simulation, "./train/result/img_char_number.bin")) {
        return 1;
    }
    
    // 加载待识别图片
    auto img = load_png_image(argv[1], INPUT_WIDTH, INPUT_HEIGHT);
    if (img.empty()) {
        std::cerr << "无法加载待识别图片" << std::endl;
        return 1;
    }
    
    // 执行识别
    int result = recognize_digit(simulation, img);
    std::cout << "识别结果: " << result << std::endl;

    // 显示识别过程的神经网络状态
    gtk_init(nullptr, nullptr);
    NeuronVisualization visualization(&simulation, SIM_WIDTH, SIM_HEIGHT);
    visualization.run();
    
    return 0;
}
