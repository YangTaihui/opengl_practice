# OpenGL_pratice

使用OpenGL进行数据可视化的示例代码。

## 内容

0_make_data：使用有限差分法生成二维和三维地震数据，可在OpenGL环境配置之前进行。

1_2d_texture：使用2D纹理绘制二维地震剖面。

2_3d_texture：使用3D纹理绘制三维地震切片。

3_3d_texture_dynamic：使用3D纹理绘制三维地震切片，使切片随时间移动，便于观察整个三维数据体。

4_2d_text：中英文混合渲染（即单双字节字符混合渲染）。

5_surface：绘制三维曲面，类似MATLAB的surface。

6_3d_ball：绘制三维球体，按照经纬度将贴图覆盖其上。

## OpenGL环境配置及运行程序

以1_2d_texture为例：

1. 下载vs2022社区版、GLAD、GLFW、GLM；

2. 打开vs2022，新建空项目，勾选将解决方案和项目放在同一目录中；

3. 将1_2d_texture文件夹内所有文件复制进解决方案所在目录；

4. 将include整个文件夹复制进解决方案所在目录；

5. 在vs2022中右键“源文件”-添加-现有项-选择“draw2d.cpp”；

6. 右击项目，单击属性；

7. 点击 C/C++ —> 常规 —> 附加包含目录，分别添加下载的 glad 和 glfw 文件夹下 include 文件夹以及glm文件夹：

   glad/include

   glfw-3.4.bin.WIN64/include

   glm-stable

8. 点击 链接器 —> 常规 —> 附加库目录，添加下载的 glfw 文件夹下 lib 文件夹：

   glfw-3.4.bin.WIN64/lib-vc2022

9. 点击 链接器 —> 输入 —> 附加依赖项，输入：

   glfw3.lib
   opengl32.lib
   user32.lib
   gdi32.lib
   shell32.lib

10. 右键“源文件”添加下载的 glad/src文件夹中的glad.c文件。

点击“开始执行”即可运行。

---

## Freetype
4_2d_text需要配置freetype，配置方法类似前文所提，Windows系统可由此[链接](https://github.com/ubawurinna/freetype-windows-binaries/releases)下载已编译好的lib和dll，将合适版本的lib和dll放入sln同一级目录，附加依赖项输入freetype.lib。
