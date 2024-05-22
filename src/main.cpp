#include <iostream>

#include <cmath>

#include <raylib/raylib.h>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

inline float randf(int a = 1000, int b = 1000){
	return float(rand() % a + 1) / b;
}

struct Color4f{
	float r, g, b, a;

	Color4f(float r, float g, float b, float a = 1.0f):
		r{r}, g{g}, b{b}, a{a}
	{}

	Color4f(float x):
		r{x}, g{x}, b{x}, a{1.0f}
	{}

	Color4f() = default;

	inline Color4f& rand(){
		r = randf(1001, 1000);
		g = randf(1001, 1000);
		b = randf(1001, 1000);
		a = 1;

		return *this;
	}

	inline Color4f operator*(const Color4f& c){
		return Color4f{
			r * c.r,
			g * c.g,
			b * c.b,
			a * c.a
		};
	}

	inline void operator*=(const Color4f& c){
		r *= c.r;
		g *= c.g;
		b *= c.b;
		a *= c.a;
	}

	inline Color4f& min(const Color4f& c){
		r = std::min(r, c.r);
		g = std::min(g, c.g);
		b = std::min(b, c.b);
		return *this;
	}

	inline Color4f& max(const Color4f& c){
		r = std::max(r, c.r);
		g = std::max(g, c.g);
		b = std::max(b, c.b);
		return *this;
	}

	inline Color4f& brighter(float x = 0.001f){
		r += x;
		g += x;
		b += x;
		return *this;
	}
};

inline float distancef(float a, float b){
	return fabsf(a - b);
}

#define PLAYER_SPACE 0.125f

inline float jumpPolynomial(float x){
	return -(x * (x - 1.f) * (x + 1.f)) * 1.5f * PLAYER_SPACE;
}

struct Point{
	Color4f col;
	Vector2 pos;

	inline Point& rand(){
		pos.y =  randf();
		col.rand() *= 0.2f;
		if(pos.y > 0.5f){
			pos.y *= 1.f + PLAYER_SPACE;
			col.g += 0.4f;
		} else{
			pos.y *= 1.f - PLAYER_SPACE;
			col.b += 0.4f;
		}
		col.max(0.1f).brighter(0.05f);
		return *this;
	}
};

#define TARGET_COUNT 6
#define POINTS_COUNT (601 + TARGET_COUNT)
Point points[POINTS_COUNT] = {};

bool drawPoints = !true;
int score = 0;
float jump_up = 0.f;
float jump_down = 0.f;

int main(int argc, const char** argv){
	srand(time(NULL));

	Point& player = points[0];
	player.col = Color4f{2.f, 0.5f, 2.f, 1.f};
	player.pos = {0.f, 0.5f};

	for(int i = 1 + TARGET_COUNT; i < POINTS_COUNT; i++){
		points[i].rand().pos.x = randf() * 3 - 1;
	}

	Point* targets[TARGET_COUNT];
	for(int i = 0; i < TARGET_COUNT; i++){
		targets[i] = &points[i + 1];
		targets[i]->col = Color4f{1.f, (i <= 3)?1.f:0.f, 0.f};
		targets[i]->pos.y = 0.5f;
		targets[i]->pos.x = randf() * 3.f - 1.f;
	}
	
	SetConfigFlags(FLAG_MSAA_4X_HINT);

	InitWindow(800, 400, "Snails");
	SetTargetFPS(60);

	Shader shader = LoadShader(TextFormat("shader/voronoi_%i.vs", GLSL_VERSION), TextFormat("shader/voronoi_%i.fs", GLSL_VERSION));

	int colors_loc = GetShaderLocation(shader, "colors");
	int positions_loc = GetShaderLocation(shader, "positions");

	while(!WindowShouldClose()){
		if(jump_up > 0.01f) jump_up -= 0.01f;
		else jump_up = 0.f;

		if(jump_down > 0.01f) jump_down -= 0.01f;
		else jump_down = 0.f;

		player.pos.y = 0.5f + jumpPolynomial(jump_down) - jumpPolynomial(jump_up);

		if(IsKeyDown(KEY_LEFT))    player.pos.x -= 0.002f;
		if(IsKeyDown(KEY_RIGHT))   player.pos.x += 0.002f;
		if(IsKeyPressed(KEY_UP))   jump_up = 1.f;
		if(IsKeyPressed(KEY_DOWN)) jump_down = 1.f;
		if(IsKeyPressed(KEY_D))    drawPoints = !drawPoints;

		float m = 0.f;
		if(player.pos.x > 0.666f){
			m = player.pos.x - 0.666f;
			player.pos.x = 0.666f;
		} else if(player.pos.x < 0.333f){
			m = player.pos.x - 0.333f;
			player.pos.x = 0.333f;
		}

		m = -m;

		SetShaderValue(shader, colors_loc, &player.col, SHADER_UNIFORM_VEC4);
		SetShaderValue(shader, positions_loc, &player.pos, SHADER_UNIFORM_VEC2);

		for(int i = 0; i < TARGET_COUNT; i++){
			targets[i]->pos.x += m;

			if(targets[i]->pos.x > 2.f){
				targets[i]->pos.x -= 2.5f + (sinf(randf()) * 0.3f);
			} else if(targets[i]->pos.x < -1.f){
				targets[i]->pos.x += 2.5f + (sinf(randf()) * 0.3f);
			}

			if(distancef(player.pos.y, targets[i]->pos.y) < 0.015f && distancef(player.pos.x, targets[i]->pos.x) < 0.015f){
				if(i <= 3) score++;
				else       score--;

				auto x = randf();
				if(x > 0.5f){
					targets[i]->pos.x = randf() + 1.f;
				} else if(x < 0.5f){
					targets[i]->pos.x = -randf();
				}
			}

			SetShaderValue(shader, colors_loc + i + 1, &targets[i]->col, SHADER_UNIFORM_VEC4);
			SetShaderValue(shader, positions_loc + i + 1, &targets[i]->pos, SHADER_UNIFORM_VEC2);
		}

		for(int i = 1 + TARGET_COUNT; i < POINTS_COUNT; i++){
			points[i].pos.x += m;

			if(points[i].pos.x > 2.f){
				points[i].rand().pos.x -= 3.f;
			} else if(points[i].pos.x < -1.f){
				points[i].rand().pos.x += 3.f;
			}

			SetShaderValue(shader, colors_loc + i, &points[i].col, SHADER_UNIFORM_VEC4);
			SetShaderValue(shader, positions_loc + i, &points[i].pos, SHADER_UNIFORM_VEC2);
		}

		BeginDrawing();
		ClearBackground({0, 0, 0, 255});

		BeginShaderMode(shader);
		DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), {0});
		EndShaderMode();

		if(drawPoints) for(auto& p: points){
			DrawCircle(p.pos.x*GetRenderWidth(), p.pos.y*GetRenderHeight(), 5, {0, 0, 0, 255});
		}

		std::string str = "Score: " + std::to_string(score);

		DrawFPS(10, 10);
		DrawText(str.c_str(), GetRenderWidth() - 150, 10, 20, {0, 150, 0, 255});

		EndDrawing();
	}
	
	UnloadShader(shader);
	CloseWindow();
	return 0;
}
