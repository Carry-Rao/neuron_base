#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <gtk/gtk.h>
#include "neuron_sim.h"

// 可视化类，负责使用GTK显示神经元模拟
class NeuronVisualization {
private:
    GtkWidget* window;
    GtkWidget* drawing_area;
    NeuralNetworkSimulation* simulation;
    gboolean running;
    guint update_timer;
    
    // 绘图回调函数
    static gboolean draw_callback(GtkWidget* widget, cairo_t* cr, gpointer data);
    
    // 模拟更新回调函数
    static gboolean update_simulation(gpointer data);
    
    // 窗口关闭回调函数
    static void on_window_closed(GtkWidget* widget, gpointer data);
    
public:
    // 构造函数
    NeuronVisualization(NeuralNetworkSimulation* sim, int width, int height);
    
    // 析构函数
    ~NeuronVisualization();
    
    // 运行主循环
    void run();
};

#endif // VISUALIZATION_H

