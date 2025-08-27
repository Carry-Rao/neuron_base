#include "visualization.h"
#include <cmath>
#include <sstream>

// 计算点(px,py)到线段(x1,y1)-(x2,y2)的最短距离
double NeuronVisualization::point_to_line_distance(double px, double py, 
                                                   double x1, double y1, 
                                                   double x2, double y2) {
    // 向量计算：线段向量AB，点到A的向量AP
    double ABx = x2 - x1;
    double ABy = y2 - y1;
    double APx = px - x1;
    double APy = py - y1;

    // 计算AP在AB上的投影比例
    double dot_product = APx * ABx + APy * ABy;
    double AB_length_sq = ABx * ABx + ABy * ABy;

    // 投影比例clamped到[0,1]（超出线段范围则取端点）
    double t = (AB_length_sq == 0) ? 0.0 : std::max(0.0, std::min(1.0, dot_product / AB_length_sq));

    // 线段上最近点的坐标
    double closest_x = x1 + t * ABx;
    double closest_y = y1 + t * ABy;

    // 计算点到最近点的距离
    double dx = px - closest_x;
    double dy = py - closest_y;
    return std::sqrt(dx * dx + dy * dy);
}

// 更新并显示突触tooltip
void NeuronVisualization::show_synapse_tooltip(int x, int y) {
    if (!hoveredSynapse.isHovered) {
        hide_synapse_tooltip();
        return;
    }

    // 构造突触信息文本（包含源/目标神经元、强度、最后使用时间）
    std::stringstream tooltip_text;
    tooltip_text << "突触信息:\n"
                 << "源神经元: " << hoveredSynapse.sourceNeuron << "\n"
                 << "目标神经元: " << hoveredSynapse.targetNeuron << "\n"
                 << "连接强度: " << std::fixed << std::setprecision(2) << hoveredSynapse.strength << "\n"
                 << "最后使用: 第" << static_cast<int>(hoveredSynapse.lastUsedStep) << "步";

    // 设置tooltip文本
    gtk_label_set_text(GTK_LABEL(tooltip_label), tooltip_text.str().c_str());
    
    // 调整tooltip窗口大小以适应文本
    gtk_widget_show_all(tooltip_window);
    gtk_window_resize(GTK_WINDOW(tooltip_window), 180, 100);

    // 将tooltip显示在鼠标右侧（避免遮挡）
    gtk_window_move(GTK_WINDOW(tooltip_window), x + 10, y + 10);
    gtk_widget_show(tooltip_window);
}

// 隐藏突触tooltip
void NeuronVisualization::hide_synapse_tooltip() {
    if (GTK_IS_WIDGET(tooltip_window) && gtk_widget_is_visible(tooltip_window)) {
        gtk_widget_hide(tooltip_window);
    }
    hoveredSynapse.isHovered = false;
}

// 鼠标移动回调：检测悬停的突触
gboolean NeuronVisualization::on_mouse_motion(GtkWidget* widget, GdkEventMotion* event, gpointer data) {
    NeuronVisualization* viz = static_cast<NeuronVisualization*>(data);
    if (!viz || !viz->simulation) return FALSE;

    const double HOVER_THRESHOLD = 8.0;  // 鼠标到突触的距离阈值（像素）
    viz->hoveredSynapse.isHovered = false;

    // 遍历所有神经元的突触
    for (size_t i = 0; i < viz->simulation->neurons.size(); ++i) {
        const auto& neuron = viz->simulation->neurons[i];
        const auto& synapses = neuron.getActiveSynapses();
        const auto& pos = neuron.getPosition();

        for (const auto& synapse : synapses) {
            const auto& target_pos = viz->simulation->neurons[synapse.targetNeuron].getPosition();
            
            // 计算鼠标到突触线段的距离
            double dist = viz->point_to_line_distance(
                event->x, event->y,
                pos.x, pos.y,
                target_pos.x, target_pos.y
            );

            // 如果距离小于阈值，记录突触信息
            if (dist < HOVER_THRESHOLD) {
                viz->hoveredSynapse.isHovered = true;
                viz->hoveredSynapse.sourceNeuron = static_cast<int>(i);
                viz->hoveredSynapse.targetNeuron = synapse.targetNeuron;
                viz->hoveredSynapse.strength = synapse.strength;
                viz->hoveredSynapse.lastUsedStep = synapse.lastUsed;
                
                // 显示tooltip
                viz->show_synapse_tooltip(static_cast<int>(event->x_root), static_cast<int>(event->y_root));
                return TRUE;
            }
        }
    }

    // 如果没有悬停在任何突触上，隐藏tooltip
    viz->hide_synapse_tooltip();
    return TRUE;
}

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
            
            // 如果是当前悬停的突触，高亮显示
            if (viz->hoveredSynapse.isHovered &&
                viz->hoveredSynapse.sourceNeuron == static_cast<int>(i) &&
                viz->hoveredSynapse.targetNeuron == synapse.targetNeuron) {
                cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);  // 黄色高亮
                cairo_set_line_width(cr, lineWidth + 1.0);       // 线宽增加
            } else {
                cairo_set_source_rgba(cr, 0.5, 0.8, 1.0, alpha); // 普通蓝色
                cairo_set_line_width(cr, lineWidth);
            }
            
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
    
    // 注册鼠标移动事件
    gtk_widget_add_events(drawing_area, GDK_POINTER_MOTION_MASK);
    g_signal_connect(drawing_area, "motion-notify-event", G_CALLBACK(on_mouse_motion), this);
    
    // 创建tooltip窗口
    tooltip_window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_resizable(GTK_WINDOW(tooltip_window), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(tooltip_window), FALSE);  // 无边框
    
    // 设置tooltip背景色
    GdkRGBA bg_color;
    gdk_rgba_parse(&bg_color, "#f0f0b0");  // 浅黄色背景
    gtk_widget_override_background_color(tooltip_window, GTK_STATE_FLAG_NORMAL, &bg_color);
    
    // 创建tooltip标签
    tooltip_label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(tooltip_label), TRUE);
    gtk_container_add(GTK_CONTAINER(tooltip_window), tooltip_label);
    
    // 初始化悬停信息
    hoveredSynapse.isHovered = false;
    hoveredSynapse.sourceNeuron = -1;
    hoveredSynapse.targetNeuron = -1;
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

