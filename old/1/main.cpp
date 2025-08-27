#include <gtk/gtk.h>
#include "neuron_sim.h"
#include "visualization.h"

int main(int argc, char* argv[]) {
    // 初始化GTK
    gtk_init(&argc, &argv);
    
    // 模拟参数
    const int NUM_NEURONS = 50;       // 神经元数量
    const int WINDOW_WIDTH = 800;     // 窗口宽度
    const int WINDOW_HEIGHT = 600;    // 窗口高度
    const double CONNECTION_THRESHOLD = 80.0;  // 连接阈值
    
    // 创建神经元模拟
    NeuralNetworkSimulation simulation(NUM_NEURONS, WINDOW_WIDTH, WINDOW_HEIGHT, CONNECTION_THRESHOLD);
    
    // 创建并运行可视化
    NeuronVisualization visualization(&simulation, WINDOW_WIDTH, WINDOW_HEIGHT);
    visualization.run();
    
    return 0;
}

