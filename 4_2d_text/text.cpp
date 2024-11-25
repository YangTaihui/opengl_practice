#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <Windows.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "../include/colormap.h"
#include "../include/functions.h"
#include "../include/shader_m.h"
#define PI 3.1415926

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 400;
const float ratio = SCR_HEIGHT / (float)SCR_WIDTH;
const float dpiw = 2.0 / SCR_WIDTH;
const float dpih = 2.0 / SCR_HEIGHT;
const float fig_x0 = -0.7;
const float fig_x1 = 0.9;
const float fig_y0 = -0.75;
const float fig_y1 = 0.85;
const float fig_w = fig_x1 - fig_x0;
const float fig_h = fig_y1 - fig_y0;
const float ticklen = 0.02;
char font_cn[] = "C:/Windows/Fonts/simsun.ttc";
char font_en[] = "C:/Windows/Fonts/times.ttf";
int font_size = 48;
float font_scale = 0.5f;

void splitGBK(std::string chars, std::vector<std::string>& words, std::vector<int>& sizes)
{
	int len = chars.length();
	int i = 0;
	int size;
	while (i < len) {
		if ((chars[i] & 0x80) == 0x00)
			size = 1;
		else
			size = 2;

		words.push_back(chars.substr(i, size));
		sizes.push_back(size);
		i += size;
	}
}

struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

class charMap {
public:
	FT_Library ft;
	FT_Face face_cn;
	FT_Face face_en;

	std::map<wchar_t, Character> Characters;
	std::map<wchar_t, Character> wCharacters;
	charMap(char* font_cn, char* font_en, int font_size);
	void clean_ft();
	void add(std::string str);
	void rend(Shader& shader, int anchor, unsigned int VAO, unsigned int VBO, std::string str, GLfloat x, GLfloat y, glm::vec3 color, GLfloat rotate = 0, GLfloat scale = font_scale);
};

charMap::charMap(char* font_cn, char* font_en, int font_size) {
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	if (FT_New_Face(ft, font_cn, 0, &face_cn))
		std::cout << "ERROR::FREETYPE: Failed to load face_cn" << std::endl;

	if (FT_New_Face(ft, font_en, 0, &face_en))
		std::cout << "ERROR::FREETYPE: Failed to load font_en" << std::endl;
	FT_Set_Pixel_Sizes(face_cn, 0, font_size);
	FT_Set_Pixel_Sizes(face_en, 0, font_size);
}

void charMap::clean_ft() {
	FT_Done_Face(face_cn);
	FT_Done_Face(face_en);
	FT_Done_FreeType(ft);
}

void charMap::add(std::string str) {
	std::vector<std::string> words;
	std::vector<int> sizes;
	splitGBK(str, words, sizes);
	//printf("ok %d %d\n", str.length(), sizes.size());

	wchar_t* ch = (wchar_t*)calloc(1, sizeof(wchar_t));
	int i;
	bool need_continue;

	for (i = 0;i < words.size();i++) {
		need_continue = false;

		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)words[i].c_str(), -1, (LPWSTR)ch, sizes[i]);

		if (sizes[i] == 1) {
			FT_Load_Char(face_en, *ch, FT_LOAD_RENDER);
			if (Characters.count(*ch) == 1)
				need_continue = true;
		}
		else {
			//printf("%d\n", *words[i].data());
			FT_Load_Char(face_cn, *ch, FT_LOAD_RENDER);
			if (wCharacters.count(*ch) == 1)
				need_continue = true;
		}
		if (need_continue)
			continue;
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		if (sizes[i] == 1) {
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face_en->glyph->bitmap.width,
				face_en->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face_en->glyph->bitmap.buffer
			);
			// Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Now store character for later use
			Character character = {
				texture,
				glm::ivec2(face_en->glyph->bitmap.width, face_en->glyph->bitmap.rows),
				glm::ivec2(face_en->glyph->bitmap_left, face_en->glyph->bitmap_top),
				face_en->glyph->advance.x
			};
			Characters.insert(std::pair<wchar_t, Character>(*ch, character));
		}
		else {
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face_cn->glyph->bitmap.width,
				face_cn->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face_cn->glyph->bitmap.buffer
			);
			// Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Now store character for later use
			Character character = {
				texture,
				glm::ivec2(face_cn->glyph->bitmap.width, face_cn->glyph->bitmap.rows),
				glm::ivec2(face_cn->glyph->bitmap_left, face_cn->glyph->bitmap_top),
				face_cn->glyph->advance.x
			};
			wCharacters.insert(std::pair<wchar_t, Character>(*ch, character));
		}
	}
}

void charMap::rend(Shader& shader, int anchor, unsigned int VAO, unsigned int VBO, std::string str, GLfloat x, GLfloat y, glm::vec3 color, GLfloat rotate, GLfloat scale) {
	// Activate corresponding render state	
	shader.setVec3("textColor", color);
	glm::mat4 one, rot, scale_glm, trans1, trans2, trans3, model;
	glm::vec3 vec_new, vec_old = glm::vec3(x, y, 0);
	one = glm::mat4(1.0f);
	rot = glm::rotate(one, glm::radians(rotate), glm::vec3(0, 0, 1.0));
	trans1 = glm::translate(one, -vec_old);
	trans2 = glm::translate(one, vec_old);

	scale_glm = glm::scale(one, glm::vec3(dpiw, dpih, 1.0));
	//model = trans1 * rot*trans2 *scale_glm;
	//shader.setMat4("model", model);
	model = scale_glm * trans2 * rot * trans1;
	shader.setMat4("model", model);
	glm::vec3 vert3lb, vert3lt, vert3rb, vert3rt;
	glm::vec4 vertlb, vertlt, vertrb, vertrt;

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	// Iterate through all characters
	GLfloat xpos, ypos, advancex = 0.0, maxh = 0.0, offset = 0.0;
	std::vector<std::string> words;
	std::vector<int> sizes;
	splitGBK(str, words, sizes);
	int i, text_len = words.size();

	wchar_t* ch = (wchar_t*)calloc(2, sizeof(wchar_t));

	Character* chs = (Character*)calloc(text_len, sizeof(Character));

	for (i = 0; i < text_len; i++) {
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)words[i].c_str(), -1, (LPWSTR)ch, sizes[i]);
		if (sizes[i] == 1) {
			chs[i] = Characters[*ch];
		}
		else
			chs[i] = wCharacters[*ch];
	}

	for (i = 0; i < text_len; i++)
	{
		maxh = (std::max)(maxh, (GLfloat)chs[i].Size.y);
		offset -= chs[i].Advance >> 6;
	}
	//std::cout << str << " " << offset << " " << text_len << std::endl;

	for (i = 0; i < text_len; i++)
	{
		if (anchor == 0) {
			xpos = x + (chs[i].Bearing.x + 0.5 * offset) * scale;
			ypos = y - (chs[i].Size.y - chs[i].Bearing.y + maxh) * scale;
		}
		else if (anchor == 2) {
			xpos = x + (chs[i].Bearing.x + offset) * scale;
			ypos = y - (chs[i].Size.y - chs[i].Bearing.y + maxh / 2) * scale;
		}
		GLfloat w = chs[i].Size.x * scale;
		GLfloat h = chs[i].Size.y * scale;

		// Update VBO for each character
		GLfloat vertices[6][5] = {
			{ xpos,     ypos + h,   0.0,  0.0, 1.0 },
			{ xpos,     ypos,       0.0,  0.0, 0.0 },
			{ xpos + w, ypos,       0.0,  1.0, 0.0 },

			{ xpos,     ypos + h,   0.0,  0.0, 1.0 },
			{ xpos + w, ypos,       0.0,  1.0, 0.0 },
			{ xpos + w, ypos + h,   0.0,  1.0, 1.0 }
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, chs[i].TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (chs[i].Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	free(chs);
	free(ch);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main()
{
	system("chcp 65001");

	int i, j, k, m;
	float x, y;
	char fname[] = "D:/RunData/2d_30001x101.bin";
	int width = 30001, height = 101, nrChannels = 3;
	int imgw = 15000, imgh = 101;
	int xlabels[] = { 0, 20, 40, 60, 80, 100 };
	std::string xlabelticks[] = { "0", "20", "40","60", "80","100" };
	std::string xlabel = "æ‡¿Î£®m£©";
	int ylabels[] = { 0, 3000, 6000, 9000, 12000, 14999 };
	std::string ylabelticks[] = { "15k", "12k", "9k","6k","3k","0" };
	std::string ylabel = " ±º‰£®s£©";

	const int xlabeln = sizeof(xlabels) / sizeof(xlabels[0]);
	const int ylabeln = sizeof(ylabels) / sizeof(ylabels[0]);
	printf("xn %d yn %d\n", xlabeln, ylabeln);

	float** data_raw0 = read2d_sample(fname, width, height, imgw, imgh);
	float** data_raw = malloc2d(imgh, imgw);
	for (i = 0; i < imgw; i++) {
		for (j = 0; j < imgh; j++) {
			data_raw[j][i] = data_raw0[i][j];
		}
	}
	i = imgw;
	imgw = imgh;
	imgh = i;

	std::cout << width << " " << height << std::endl;
	unsigned char* data = (unsigned char*)calloc(imgh * imgw * nrChannels, sizeof(unsigned char));
	std::cout << "datas " << imgh * imgw * nrChannels << std::endl;
	float maxv = 0.0, minv = 0.0;
	min_max_2d(data_raw, imgw, imgh, 0.5);

	int now_id, now_v;
	for (j = 0; j < imgh; j++) {
		for (i = 0; i < imgw; i++) {
			now_id = (j * imgw + i) * 3;
			now_v = round(data_raw[i][j]);
			if (now_v < 0)
				now_v = 0;
			data[now_id] = colormap_promax[now_v][0];
			data[now_id + 1] = colormap_promax[now_v][1];
			data[now_id + 2] = colormap_promax[now_v][2];
		}
	}

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// positions          // texture coords
	   fig_x1, fig_y1, 0.0f,   1.0f, 1.0f, // top right
	   fig_x1, fig_y0, 0.0f,   1.0f, 0.0f, // bottom right
	   fig_x0, fig_y0, 0.0f,   0.0f, 0.0f, // bottom left
	   fig_x0, fig_y1, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		3, 2, 1, // first triangle
		3, 1, 0  // second triangle
	};

	float** vertices1_3 = malloc2d(8 + (xlabeln + ylabeln) * 2, 3);
	// outlines
	float vertices_outlines[] = {
		fig_x1, fig_y1, 0.0f,
		fig_x1, fig_y0, 0.0f,
		fig_x1, fig_y0, 0.0f,
		fig_x0, fig_y0, 0.0f,
		fig_x0, fig_y0, 0.0f,
		fig_x0, fig_y1, 0.0f,
		fig_x0, fig_y1, 0.0f,
		fig_x1, fig_y1, 0.0f,
	};

	for (i = 0; i < 8;i++) {
		vertices1_3[i][0] = vertices_outlines[3 * i];
		vertices1_3[i][1] = vertices_outlines[3 * i + 1];
		vertices1_3[i][2] = vertices_outlines[3 * i + 2];
	}

	for (i = 0; i < xlabeln;i++) {
		vertices1_3[8 + i * 2][0] = fig_w * xlabels[i] / imgw + fig_x0;
		vertices1_3[8 + i * 2][1] = fig_y0;
		vertices1_3[8 + i * 2][2] = 0;
		vertices1_3[8 + i * 2 + 1][0] = fig_w * xlabels[i] / imgw + fig_x0;
		vertices1_3[8 + i * 2 + 1][1] = fig_y0 - ticklen;
		vertices1_3[8 + i * 2 + 1][2] = 0;
	}
	j = 8 + xlabeln * 2;
	for (i = 0; i < ylabeln;i++) {
		vertices1_3[j + i * 2][0] = fig_x0;
		vertices1_3[j + i * 2][1] = fig_h * ylabels[i] / imgh + fig_y0;
		vertices1_3[j + i * 2][2] = 0;
		vertices1_3[j + i * 2 + 1][0] = fig_x0 - ticklen;
		vertices1_3[j + i * 2 + 1][1] = fig_h * ylabels[i] / imgh + fig_y0;
		vertices1_3[j + i * 2 + 1][2] = 0;
	}
	float* vertices1 = calloc1d_float(24 + (xlabeln + ylabeln) * 6);
	k = 0;
	for (i = 0;i < 8 + (xlabeln + ylabeln) * 2;i++) {
		for (j = 0;j < 3;j++) {
			vertices1[k] = vertices1_3[i][j];
			k += 1;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	charMap charmap(font_cn, font_en, font_size);
	std::string texts;
	for (i = 0; i < xlabeln; i++) {
		texts += xlabelticks[i];
	}

	for (i = 0; i < ylabeln; i++) {
		texts += ylabelticks[i];
	}
	texts = texts + xlabel + ylabel;

	charmap.add(texts);

	//glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	charmap.clean_ft();

	unsigned int VAO[3], VBO[3], EBO[2];
	glGenVertexArrays(3, VAO);
	glGenBuffers(3, VBO);
	glGenBuffers(2, EBO);

	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, (24 + (xlabeln + ylabeln) * 6) * sizeof(float), vertices1, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 30, NULL, GL_DYNAMIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set OpenGL options
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// load and create a texture 
	// -------------------------
	unsigned int texture1;
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLint textureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &textureSize);
	std::cout << "GL_MAX_TEXTURE_SIZE " << textureSize << std::endl;

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgw, imgh, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("./text.vs", "./text.fs");
	Shader ourShader1("./line.vs", "./line.fs");
	glm::mat4 model = glm::mat4(1.0f);
	ourShader.use();
	ourShader.setInt("texture1", 1);
	ourShader.setInt("text1", 0);

	glm::vec3 color1 = glm::vec3(0.0, 0.0f, 0.0f);
	glm::vec3 color2 = glm::vec3(0.5, 0.5f, 0.5f);

	// render loop
	// -----------

	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.7f, 0.9f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ourShader.use();
		ourShader.setMat4("model", model);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);

		ourShader.setBool("istext", false);
		glBindVertexArray(VAO[0]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		ourShader.setBool("istext", true);
		charmap.rend(ourShader, 0, VAO[2], VBO[2], xlabel, (fig_w * 0.5 + fig_x0) / dpiw, (fig_y0 - 4 * ticklen) / dpih, color1, 0);
		for (i = 0;i < xlabeln;i++) {
			charmap.rend(ourShader, 0, VAO[2], VBO[2], xlabelticks[i], (fig_w * xlabels[i] / (float)imgw + fig_x0) / dpiw, (fig_y0 - ticklen) / dpih, color1);
		}
		charmap.rend(ourShader, 0, VAO[2], VBO[2], ylabel, (fig_y0 - 10 * ticklen) / dpiw, (fig_h * 0.5 + fig_y0) / dpih, color1, 90);
		for (i = 0;i < ylabeln;i++) {
			charmap.rend(ourShader, 2, VAO[2], VBO[2], ylabelticks[i], (fig_x0 - ticklen) / dpiw, (fig_h * ylabels[i] / (float)imgh + fig_y0) / dpih, color2, 15);
		}

		ourShader1.use();
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_LINES, 0, 8 + (xlabeln + ylabeln) * 2);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(3, VAO);
	glDeleteBuffers(3, VBO);
	glDeleteBuffers(2, EBO);
	//free(data);
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	float new_ratio = height / (float)width;
	if (new_ratio > ratio)
		glViewport(0, abs(height - width * ratio) / 2, width, width * ratio);
	else if (new_ratio < ratio)
		glViewport(abs(width - height / ratio) / 2, 0, height / ratio, height);
	else
		glViewport(0, 0, width, height);

}

//wchar_t* string2wchart(std::string str) {
//    int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str.c_str(), -1, NULL, 0);
//    wchar_t* wszUtf8 = new wchar_t[len + 1];
//    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str.c_str(), -1, (LPWSTR)wszUtf8, len);
//    return wszUtf8;
//}
//
//char* string2char(std::string str) {
//    char* p = (char*)str.c_str();
//    return p;
//}