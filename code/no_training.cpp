#include "neuron_sim.h"
#include "visualization.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>

// CLI模式运行函数
void run_cli_mode(int num_neurons, double width, double height, double threshold, bool output = true) {
    // 创建神经元模拟实例
    NeuralNetworkSimulation simulation(num_neurons, width, height, threshold);
    
    std::cout << "=== 神经元模拟 (CLI模式) ===" << std::endl;
    std::cout << "神经元数量: " << num_neurons << std::endl;
    std::cout << "空间大小: " << width << "x" << height << std::endl;
    std::cout << "连接阈值: " << threshold << std::endl << std::endl;

    // 记录开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 运行模拟
    for (int step = 0; ; ++step) {
        simulation.step();
        
	step %= 100;

        // 定期输出进度
        if (step == 0 && output) {
            std::cout << "总突触数: " << std::setw(6) << simulation.getTotalSynapses()
                      << " | 发放神经元数: " << simulation.getFiringCount() << std::endl;
        }
    }

    // 计算并输出总耗时
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << std::endl << "模拟完成! 总耗时: " << elapsed.count() << " 秒" << std::endl;
}

// GUI模式运行函数
void run_gui_mode(int num_neurons, double width, double height, double threshold) {
    // 初始化GTK
    gtk_init(nullptr, nullptr);
    
    // 创建神经元模拟
    NeuralNetworkSimulation simulation(num_neurons, width, height, threshold);
    
    // 创建并运行可视化
    NeuronVisualization visualization(&simulation, width, height);
    visualization.run();
}

int main(int argc, char* argv[]) {
    // 默认参数
    int num_neurons = 200;
    double width = 800.0;
    double height = 600.0;
    double threshold = 80.0;

    // 解析命令行参数
    bool use_gui = true; // 默认GUI模式
    bool output = true;

    // 根据模式运行
    if (use_gui) {
        run_gui_mode(num_neurons, width, height, threshold);
    } else {
        run_cli_mode(num_neurons, width, height, threshold, output);
    }

    return 0;
}

