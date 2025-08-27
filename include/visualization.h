#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <gtk/gtk.h>
#include <iomanip>
#include "neuron_sim.h"

// 存储当前悬停的突触信息
struct HoveredSynapseInfo {
    bool isHovered;          // 是否悬停在突触上
    int sourceNeuron;        // 源神经元索引
    int targetNeuron;        // 目标神经元索引
    double strength;         // 突触强度
    double lastUsedStep;     // 最后使用的模拟步数
};

class NeuronVisualization {
public:
    // 构造函数：传入模拟对象、窗口宽高
    NeuronVisualization(NeuralNetworkSimulation* sim, int width, int height);
    // 析构函数：释放资源
    ~NeuronVisualization();
    // 运行GUI主循环
    void run();

private:
    NeuralNetworkSimulation* simulation;  // 神经元模拟对象指针
    GtkWidget* window;                    // 主窗口
    GtkWidget* drawing_area;              // 绘图区域
    GtkWidget* tooltip_window;            // 突触信息提示窗口
    GtkWidget* tooltip_label;             // 提示窗口中的文本标签
    bool running;                         // 模拟是否正在运行
    guint update_timer;                   // 模拟更新定时器ID
    HoveredSynapseInfo hoveredSynapse;    // 当前悬停的突触信息

    // 绘图回调函数：绘制神经元和突触
    static gboolean draw_callback(GtkWidget* widget, cairo_t* cr, gpointer data);
    // 模拟更新回调函数：每帧更新模拟状态
    static gboolean update_simulation(gpointer data);
    // 窗口关闭回调函数：停止模拟
    static void on_window_closed(GtkWidget* widget, gpointer data);
    // 鼠标移动回调函数：检测是否悬停在突触上
    static gboolean on_mouse_motion(GtkWidget* widget, GdkEventMotion* event, gpointer data);
    
    // 辅助函数：计算点到线段的距离（判断鼠标是否靠近突触）
    double point_to_line_distance(double px, double py, 
                                 double x1, double y1, 
                                 double x2, double y2);
    // 辅助函数：更新tooltip内容并显示
    void show_synapse_tooltip(int x, int y);
    // 辅助函数：隐藏tooltip
    void hide_synapse_tooltip();
};

#endif // VISUALIZATION_H

