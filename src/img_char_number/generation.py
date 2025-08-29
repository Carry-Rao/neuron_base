import os
import glob
from PIL import Image, ImageDraw, ImageFont

# -------------------------- 配置参数 --------------------------
IMAGE_SIZE = (200, 200)  # 图片尺寸（宽，高）
BACKGROUND_COLOR = (255, 255, 255)  # 背景色（白色）
TEXT_COLOR = (0, 0, 0)  # 文字颜色（黑色）
FONT_SIZE = 120  # 字体大小
BASE_FOLDER = "./train/img/char/number"  # 保存基础路径
SAMPLES_PER_NUMBER = 1  # 每种字体为每个数字生成的样本数

# Linux系统常见的字体安装路径
FONT_PATHS = [
    "/usr/share/fonts/",
    "~/.local/share/fonts/",
    "~/.fonts/",
]

# 支持的字体文件扩展名
FONT_EXTENSIONS = ['.ttf', '.otf', '.ttc', '.woff', '.woff2']

# -------------------------- 工具函数 --------------------------
def get_all_system_fonts():
    """获取系统中所有可用的字体文件路径"""
    font_files = []
    
    for path in FONT_PATHS:
        # 扩展用户目录（处理~符号）
        expanded_path = os.path.expanduser(path)
        
        if not os.path.exists(expanded_path):
            continue
            
        # 递归查找所有字体文件
        for ext in FONT_EXTENSIONS:
            # 使用glob递归查找
            pattern = os.path.join(expanded_path, f'**/*{ext}')
            found_files = glob.glob(pattern, recursive=True)
            font_files.extend(found_files)
    
    # 去重并返回
    return list(set(font_files))

# -------------------------- 生成逻辑 --------------------------
if __name__ == "__main__":
    # 获取系统中所有字体
    print("正在扫描系统字体...")
    system_fonts = get_all_system_fonts()
    print(f"找到 {len(system_fonts)} 个字体文件")
    
    if not system_fonts:
        print("未找到任何字体文件，请检查系统字体安装情况")
        exit(1)
    
    # 循环生成0-9每个数字
    for num in range(10):
        # 创建数字对应的文件夹
        num_folder = os.path.join(BASE_FOLDER, str(num))
        os.makedirs(num_folder, exist_ok=True)
        
        # 用于记录该数字的样本编号
        sample_count = 0
        
        # 循环每种字体
        for font_path in system_fonts:
            try:
                # 尝试加载字体
                font = ImageFont.truetype(font_path, FONT_SIZE)
                
                # 生成指定数量的样本
                for _ in range(SAMPLES_PER_NUMBER):
                    sample_count += 1
                    # 创建空白图片
                    img = Image.new("RGB", IMAGE_SIZE, BACKGROUND_COLOR)
                    draw = ImageDraw.Draw(img)
                    
                    # 计算文字居中位置
                    text_bbox = draw.textbbox((0, 0), str(num), font=font)
                    text_width = text_bbox[2] - text_bbox[0]
                    text_height = text_bbox[3] - text_bbox[1]
                    x = (IMAGE_SIZE[0] - text_width) // 2
                    y = (IMAGE_SIZE[1] - text_height) // 2
                    
                    # 绘制数字
                    draw.text((x, y), str(num), font=font, fill=TEXT_COLOR)
                    
                    # 保存图片
                    save_path = os.path.join(num_folder, f"{sample_count}.png")
                    img.save(save_path)
                    print(f"已生成：{save_path}")
        
            except Exception as e:
                # 忽略无法使用的字体文件
                print(f"跳过不可用的字体 {font_path}：{str(e)}")
                continue

    print("所有数字图片生成完成！")
    
