using OpenTK.Mathematics;
using OpenTK.Windowing.GraphicsLibraryFramework;
using OpenTK.Graphics.OpenGL;
using System;
using System.Collections.Generic;

namespace Shard.HybridVoxelGL.Games
{
    public class DemoGame : Game
    {
        // My objects
        Cube cube1, cube2;
        Octree octree1, octree2;

        List<RastyObject> myCubes = [];
        ObjVolume cow;

        private float time;
        Camera cam;
        Vector2 lastMousePos;

        PostProcess mosaicPost;
        PostProcess crtPost;

        public override void Init()
        {
            string masterBank = Bootystrap.Instance.AssetManager.getAssetPath("Master.bank");
            string musicStringBank = Bootystrap.Instance.AssetManager.getAssetPath("Master.strings.bank");
            string musicBank = Bootystrap.Instance.AssetManager.getAssetPath("Music.bank");

            Bootystrap.Instance.SoundFMOD.LoadSoundAssets(masterBank);
            Bootystrap.Instance.SoundFMOD.LoadSoundAssets(musicStringBank);
            Bootystrap.Instance.SoundFMOD.LoadSoundAssets(musicBank);

            cube1 = new Cube();
            cube2 = new Cube();
            cube1.Scale = Vector3.One * 1.2f;
            cube2.Scale = Vector3.One * 0.7f;
            SceneObjects.Add(cube1);
            SceneObjects.Add(cube2);


            // octree1 = new Octree();
            // octree2 = new Octree();
            // octree1.Scale = Vector3.One * 0.25f;
            // octree2.Scale = Vector3.One * 0.65f;
            // SceneObjects.Add(octree1);
            // SceneObjects.Add(octree2);

            cow = ObjVolume.LoadFromFile(Bootystrap.Instance.AssetManager.getAssetPath("cow.obj"));
            cow.Scale *= 0.2f;
            cow.Position += new Vector3(0, -2f, 5);
            cow.SetColor(new(0.7f, 0.3f, 0.4f));
            SceneObjects.Add(cow);

            mosaicPost = new PostProcess("mosaicPostProcess.frag");
            crtPost = new PostProcess("crtPostProcess.frag");
            PostProcesses.Add(mosaicPost);
            PostProcesses.Add(crtPost);

            for (int i = 0; i < 50; i++)
            {
                var cube = new Cube();
                cube.Scale *= 0.15f;
                cube.Position = new Vector3(Random.Shared.Next(-60, 60) / 10.0f, Random.Shared.Next(-5, 5) / 10.0f - 1, Random.Shared.Next(-60, 60) / 10.0f);
                cube.SetColor(new Vector3((float)Random.Shared.NextDouble(), (float)Random.Shared.NextDouble(), (float)Random.Shared.NextDouble()));

                myCubes.Add(cube);
                SceneObjects.Add(cube);
            }

            cam = Bootystrap.Instance.Camera;

            lastMousePos = new Vector2(-1);
        }

        protected override void Update(float deltaTime)
        {
            //Move objects
            time += deltaTime;

            ProcessInput(deltaTime);

            cube1.Position = new Vector3(1f, -0.5f + (float)Math.Sin(time), -3.0f);
            cube1.Rotation = new Vector3(0.55f * time, 0.25f * time, 0);

            cube2.Position = new Vector3(-1f, 0.5f + (float)Math.Cos(time), -2.0f);
            cube2.Rotation = new Vector3(-0.25f * time, -0.35f * time, 0);

            // octree1.Position = new Vector3(3f, -0.5f + (float)Math.Sin(time), -3.0f);
            // octree2.Position = new Vector3(0f, -0.5f + (float)Math.Sin(time), -3.0f);

            cow.Position = new Vector3(-3f, -0.5f + (float)Math.Cos(time), -2.0f);
            cow.Rotation = new Vector3(-0.25f * time, -0.35f * time, 0);

            for (int i = 0; i < myCubes.Count; i++)
            {
                myCubes[i].Rotation = new Vector3(0.55f * time + i, 0.25f * time + i, 0);
            }

            // Update shaders
            GL.Uniform1(crtPost.GetUniformLocation("time"), time);
        }

        private void ProcessInput(float deltaTime)
        {
            if (KeyboardState.IsKeyDown(Keys.Escape)) Close();

            var camMove = Vector3.Zero;
            if (KeyboardState.IsKeyDown(Keys.W))
                camMove += Vector3.UnitZ;
            if (KeyboardState.IsKeyDown(Keys.S))
                camMove -= Vector3.UnitZ;
            if (KeyboardState.IsKeyDown(Keys.A))
                camMove -= Vector3.UnitX;
            if (KeyboardState.IsKeyDown(Keys.D))
                camMove += Vector3.UnitX;
            if (KeyboardState.IsKeyDown(Keys.Q))
                camMove += Vector3.UnitY;
            if (KeyboardState.IsKeyDown(Keys.E))
                camMove -= Vector3.UnitY;
            cam.Move(camMove, (float)deltaTime, KeyboardState.IsKeyDown(Keys.LeftShift));

            if (KeyboardState.IsKeyDown(Keys.M))
                Bootystrap.Instance.SoundFMOD.playSound("event:/Character/Cow");

            if (IsFocused)
            {
                var mouseDelta = lastMousePos - new Vector2(MouseState.X, MouseState.Y);
                cam.AddRotation(mouseDelta);
                lastMousePos = new Vector2(MouseState.X, MouseState.Y);
            }

            if (KeyboardState.IsKeyDown(Keys.R))
                cam.Direction = -Vector3.UnitZ;
            if (KeyboardState.IsKeyDown(Keys.T))
                cam.Position = Vector3.Zero;
        }
    }
}
