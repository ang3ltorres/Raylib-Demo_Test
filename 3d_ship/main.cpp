#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include <new>

struct BULLET
{
	BULLET()
	{
		enable = false;
	}

	BULLET(Vector3 pos, Vector3 direction, float speed)
	{
		enable = true;
		timer = 60*2;

		this->pos = pos;
		this->direction = direction;
		this->speed = speed;
	}

	void Update()
	{
		if (timer > 0) timer--;
		else enable = false;

		pos.x += direction.x * speed;
		pos.y += direction.y * speed;
		pos.z += direction.z * speed;
	}

	void Draw()
	{
		DrawSphere(pos, 0.1, GREEN);
	}

	bool enable;
	unsigned int timer;

	Vector3 pos;
	Vector3 direction;
	float speed;
};

struct SHIP
{
	SHIP(Model* model)
	{
		this->model = model;
		pos = (Vector3){0, 0, 0};
		rot = (Vector3){0, 0, 0}; // yaw pitch roll
		speed = 0.2;

		bullet = (BULLET*)::operator new(sizeof(BULLET)*16);
		for (int i = 0; i < 16; i++) new(&bullet[i]) BULLET();
	}

	~SHIP()
	{
		::operator delete(bullet);
	}

	void Update()
	{
		// Get direction vector from (yaw, pitch, roll)
		direction = (Vector3){0, 0, 0};
		direction.x = cos(DEG2RAD * rot.z) * cos(DEG2RAD * -rot.x);
		direction.z = sin(DEG2RAD * rot.z) * cos(DEG2RAD * -rot.x);
		direction.y = sin(DEG2RAD * -rot.x);

		// Shoot
		if (IsKeyPressed(KEY_LEFT_SHIFT))
		{
			for (int i = 0; i < 16; i++)
			{
				if (not bullet[i].enable)
				{
					new(&bullet[i]) BULLET(pos, direction, 0.5);
					break;
				}
			}
		}

		// Accelerate
		if (IsKeyDown(KEY_SPACE))
		{
			// Apply movement
			pos.x += direction.x * speed;
			pos.y += direction.y * speed;
			pos.z += direction.z * speed;
		}

		// Change direction
		if (IsKeyDown('D')) rot.z += 1;
		if (IsKeyDown('A')) rot.z -= 1;

		if (IsKeyDown('W')) rot.x += 1;
		if (IsKeyDown('S')) rot.x -= 1;

		// Update bullets
		for (int i = 0; i < 16; i++) if (bullet[i].enable) bullet[i].Update();
	}

	void Draw()
	{
		// RAW PITCH ROLL relative to world
		//model.transform = MatrixRotateXYZ((Vector3){DEG2RAD*rot.y, DEG2RAD*rot.z, DEG2RAD*rot.x});

		// RAW PITCH ROLL relative to model
		Matrix transform = MatrixIdentity();
		transform = MatrixMultiply(transform, MatrixRotateZ(DEG2RAD*rot.x));
		transform = MatrixMultiply(transform, MatrixRotateX(DEG2RAD*rot.y));
		transform = MatrixMultiply(transform, MatrixRotateY(DEG2RAD*rot.z));
		model->transform = transform;
		
		DrawModel(*model, pos, 1, WHITE);

		// Draw bullets
		for (int i = 0; i < 16; i++) if (bullet[i].enable) bullet[i].Draw();
	}

	Model* model;
	Vector3 pos;
	Vector3 rot;
	float speed;
	Vector3 direction;
	BULLET* bullet;
};

struct CAM
{
	CAM(SHIP* ship)
	{
		follow = false;
		camera = (Camera){(Vector3){10, 4, 10}, (Vector3){0, 0, 0}, (Vector3){0, 1, 0}, 45, CAMERA_PERSPECTIVE};
		camera_angle = 0;
		camera_radius = 15;

		this->ship = ship;
	}

	void Update()
	{
		if (follow)
		{
			camera.target = ship->pos;
			camera.position = ship->pos;
			
			// Tell to the camera where is "up" (relative to ship)
			// Get direction vector from (yaw, pitch, roll)
			camera.up = (Vector3){0, 0, 0};
			camera.up.x = cos(DEG2RAD * ship->rot.z) * cos(DEG2RAD * (-ship->rot.x + 90));
			camera.up.z = sin(DEG2RAD * ship->rot.z) * cos(DEG2RAD * (-ship->rot.x + 90));
			camera.up.y = sin(DEG2RAD * (-ship->rot.x + 90));
			
			// Move the camera to ship rear
			camera.position.x += ship->direction.x * -15;
			camera.position.y += ship->direction.y * -15;
			camera.position.z += ship->direction.z * -15;
		}
		else
		{
			camera.target = (Vector3){0, 0, 0};

			// Tell to the camera where is "up" (relative to world)
			camera.up = (Vector3){0, 1, 0};
			camera.position = (Vector3){10, 4, 10};

			// Rotate
			if (IsKeyDown('E')) camera_angle += 1;
			if (IsKeyDown('Q')) camera_angle -= 1;
			
			// Zoom in/out
			if (IsKeyDown(KEY_KP_ADD)) camera_radius--;
			if (IsKeyDown(KEY_KP_SUBTRACT)) camera_radius++;

			// Rotate camera
			camera.position.x = 0 + camera_radius * sin(DEG2RAD * camera_angle);
			camera.position.z = 0 + camera_radius * cos(DEG2RAD * camera_angle);
		}
	}

	bool follow;
	Camera camera;
	float camera_angle;
	float camera_radius;

	SHIP* ship;
};

int main()
{
	SetConfigFlags(FLAG_FULLSCREEN_MODE);
	InitWindow(1920, 1080, "3D SHIP");
	rlDisableBackfaceCulling();

	Model test = LoadModel("data/ship.gltf");
	Model moon = LoadModel("data/moon.gltf");

	SHIP ship = SHIP(&test);

	CAM cam = CAM(&ship);
	SetCameraMode(cam.camera, CAMERA_CUSTOM);

	while (!WindowShouldClose())
	{
		// UPDATE
		ship.Update();
		cam.Update();

		// Camera follow ship / Camera rotate relative to world origin
		if (IsKeyPressed(KEY_LEFT_CONTROL)) cam.follow = not cam.follow;

		// DRAW
		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode3D(cam.camera);

			DrawGrid(32, 1);

			ship.Draw();
			DrawModel(moon, (Vector3){0, 50, 0}, 10, WHITE);

			DrawSphere((Vector3){0, 0, 0}, 0.1, RED);

		EndMode3D();

		DrawFPS(0, 0);
		EndDrawing();
	}

	UnloadModel(test);
	UnloadModel(moon);
	CloseWindow();
	return 0;
}