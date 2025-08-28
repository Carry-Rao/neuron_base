#include "neuron_sim.h"
#include "visualization.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fs = std::filesystem;

// 加载PNG图片并转换为灰度值（0-1）
std::vector<std::vector<double>> load_png_image(const std::string& path, int target_width, int target_height) {
    std::vector<std::vector<double>> img;
    int width, height, channels;
    
    // 加载图片（自动转换为灰度图）
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 1);
    if (!data) {
        std::cerr << "无法加载图片: " << path << " (" << stbi_failure_reason() << ")" << std::endl;
        return img;
    }
    
    // 缩放图片到目标尺寸（最近邻缩放）
    img.resize(target_height, std::vector<double>(target_width, 0.0));
    double x_ratio = static_cast<double>(width) / target_width;
    double y_ratio = static_cast<double>(height) / target_height;
    
    for (int y = 0; y < target_height; ++y) {
        for (int x = 0; x < target_width; ++x) {
            int src_x = static_cast<int>(x * x_ratio);
            int src_y = static_cast<int>(y * y_ratio);
            int idx = src_y * width + src_x;
            // 反转颜色（假设黑色背景白色数字）
            img[y][x] = 1.0 - (static_cast<double>(data[idx]) / 255.0);
        }
    }
    
    stbi_image_free(data);
    return img;
}

// 保存训练结果
void save_training_result(const NeuralNetworkSimulation& sim, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开文件保存训练结果: " << path << std::endl;
    }
    
    // 保存神经元数量
    size_t num_neurons = sim.neurons.size();
    file.write(reinterpret_cast<const char*>(&num_neurons), sizeof(num_neurons));
    
    // 保存每个神经元的连接信息
    for (const auto& neuron : sim.neurons) {
        const auto& synapses = neuron.getActiveSynapses();
        size_t num_synapses = synapses.size();
        file.write(reinterpret_cast<const char*>(&num_synapses), sizeof(num_synapses));
        
        for (const auto& synapse : synapses) {
            file.write(reinterpret_cast<const char*>(&synapse.targetNeuron), sizeof(synapse.targetNeuron));
            file.write(reinterpret_cast<const char*>(&synapse.strength), sizeof(synapse.strength));
            file.write(reinterpret_cast<const char*>(&synapse.lastUsed), sizeof(synapse.lastUsed));
        }
    }
    
    std::cout << "训练结果已保存到: " << path << std::endl;
}

int main() {
    const int SIM_WIDTH = 1000;
    const int SIM_HEIGHT = 800;
    const int INPUT_WIDTH = 28;   // MNIST格式大小
    const int INPUT_HEIGHT = 28;
    const int NUM_NEURONS = INPUT_WIDTH * INPUT_HEIGHT + 10;  // 输入层 + 10个输出神经元
    const double THRESHOLD = 150;
    const int TRAIN_STEPS = 1000;
    
    // 创建神经网络模拟
    NeuralNetworkSimulation simulation(NUM_NEURONS, SIM_WIDTH, SIM_HEIGHT, THRESHOLD);
    std::cout << "初始化神经网络，神经元数量: " << NUM_NEURONS << std::endl;

    // 训练0-9数字
    for (int digit = 0; digit < 10; ++digit) {
        std::cout << "训练数字: " << digit << std::endl;
        std::string img_dir = "./train/img/char/number/" + std::to_string(digit);
        
        if (!fs::exists(img_dir)) {
            std::cerr << "训练目录不存在: " << img_dir << std::endl;
            continue;
        }
        
        // 加载该数字的所有PNG图片
        int img_count = 0;
        for (const auto& entry : fs::directory_iterator(img_dir)) {
            if (entry.path().extension() == ".png") {
                auto img = load_png_image(entry.path(), INPUT_WIDTH, INPUT_HEIGHT);
                if (img.empty()) {
                    std::cerr << "无效的图片: " << entry.path() << std::endl;
                    continue;
                }
                
                img_count++;
                // 激活输入层神经元（前28*28个神经元）
                for (int y = 0; y < INPUT_HEIGHT; ++y) {
                    for (int x = 0; x < INPUT_WIDTH; ++x) {
                        int neuron_idx = y * INPUT_WIDTH + x;
                        if (img[y][x] > 0.5 && neuron_idx < simulation.neurons.size()) {
                            simulation.neurons[neuron_idx].fire(simulation.currentStep);
                        }
                    }
                }
                
                // 激活对应数字的输出神经元（最后10个神经元）
                int output_neuron = INPUT_WIDTH * INPUT_HEIGHT + digit;
                if (output_neuron < simulation.neurons.size()) {
                    simulation.neurons[output_neuron].fire(simulation.currentStep);
                }
                
                // 运行训练步骤
                for (int step = 0; step < TRAIN_STEPS; ++step) {
                    simulation.step();
                    if (step % 100 == 0) {
                        std::cout << "\r训练进度: " << (step * 100 / TRAIN_STEPS) << "% " << std::flush;
                    }
                }
                std::cout << "\r训练进度: 100% 完成" << std::endl;
            }
        }
        
        std::cout << "数字 " << digit << " 训练完成，处理图片数量: " << img_count << std::endl;
    }
    
    // 保存训练结果
    save_training_result(simulation, "./train/result/img_char_number.bin");

    // // 可视化训练后的网络
    // std::cout << "显示训练后的神经网络..." << std::endl;
    // gtk_init(nullptr, nullptr);
    // NeuronVisualization visualization(&simulation, SIM_WIDTH, SIM_HEIGHT);
    // visualization.run();
    
    return 0;
}
