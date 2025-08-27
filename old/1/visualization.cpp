#include "visualization.h"

// 绘图回调函数实现
gboolean NeuronVisualization::draw_callback(GtkWidget* widget, cairo_t* cr, gpointer data) {
    NeuronVisualization* viz = static_cast<NeuronVisualization*>(data);
    if (!viz || !viz->simulation) return FALSE;
    
    gint width = gtk_widget_get_allocated_width(widget);
    gint height = gtk_widget_get_allocated_height(widget);
    
    // 绘制背景
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_paint(cr);
    
    // 绘制突触连接
    for (size_t i = 0; i < viz->simulation->neurons.size(); ++i) {
        const auto& neuron = viz->simulation->neurons[i];
        const auto& synapses = neuron.getActiveSynapses();
        const auto& pos = neuron.getPosition();
        
        for (const auto& synapse : synapses) {
            const auto& targetPos = viz->simulation->neurons[synapse.targetNeuron].getPosition();
            
            // 突触强度决定线条透明度和宽度
            double alpha = synapse.strength;
            double lineWidth = 0.5 + (synapse.strength * 1.5);
            
            cairo_set_source_rgba(cr, 0.5, 0.8, 1.0, alpha);
            cairo_set_line_width(cr, lineWidth);
            
            cairo_move_to(cr, pos.x, pos.y);
            cairo_line_to(cr, targetPos.x, targetPos.y);
            cairo_stroke(cr);
        }
    }
    
    // 绘制神经元
    for (const auto& neuron : viz->simulation->neurons) {
        const auto& pos = neuron.getPosition();
        double activation = neuron.getActivationLevel();
        
        // 激活水平决定颜色
        if (neuron.firing()) {
            // 正在发放的神经元显示为红色
            cairo_set_source_rgb(cr, 1.0, 0.2, 0.2);
        } else {
            // 激活水平高的神经元显示为更亮的蓝色
            cairo_set_source_rgb(cr, 0.3, 0.5 + activation * 0.5, 1.0);
        }
        
        // 绘制神经元主体
        double radius = 5.0 + (activation * 3.0);
        cairo_arc(cr, pos.x, pos.y, radius, 0, 2 * G_PI);
        cairo_fill(cr);
        
        // 绘制神经元边框
        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
        cairo_set_line_width(cr, 1.0);
        cairo_arc(cr, pos.x, pos.y, radius, 0, 2 * G_PI);
        cairo_stroke(cr);
    }
    
    return FALSE;
}

// 模拟更新回调函数实现
gboolean NeuronVisualization::update_simulation(gpointer data) {
    NeuronVisualization* viz = static_cast<NeuronVisualization*>(data);
    if (!viz || !viz->running || !viz->simulation) return FALSE;
    
    viz->simulation->step();
    
    // 重绘窗口
    gtk_widget_queue_draw(viz->drawing_area);
    
    return TRUE;
}

// 窗口关闭回调函数实现
void NeuronVisualization::on_window_closed(GtkWidget* widget, gpointer data) {
    NeuronVisualization* viz = static_cast<NeuronVisualization*>(data);
    if (viz) {
        viz->running = FALSE;
    }
}

// 构造函数实现
NeuronVisualization::NeuronVisualization(NeuralNetworkSimulation* sim, int width, int height)
    : simulation(sim), running(FALSE), update_timer(0) {
    // 初始化GTK窗口
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "神经元网络模拟");
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_closed), this);
    
    // 创建绘图区域
    drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_callback), this);
}

// 析构函数实现
NeuronVisualization::~NeuronVisualization() {
    if (update_timer) {
        g_source_remove(update_timer);
    }
}

// 运行主循环
void NeuronVisualization::run() {
    running = TRUE;
    // 设置定时器，每50毫秒更新一次模拟
    update_timer = g_timeout_add(50, update_simulation, this);
    
    // 显示所有控件
    gtk_widget_show_all(window);
    
    // 运行主循环
    gtk_main();
}

