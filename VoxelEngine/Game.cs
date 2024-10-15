using System;
using OpenTK.Windowing.Common;
using OpenTK.Windowing.Desktop;
using OpenTK.Graphics.OpenGL;
using OpenTK.Mathematics;
using System.Collections.Generic;
using System.Threading.Tasks;


namespace Shard.HybridVoxelGL
{
    public class Game : GameWindow
    {
        public List<GameObject> SceneObjects = [];
        public List<PostProcess> PostProcesses = [];

        public int Width { get; private set; }
        public int Height { get; private set; }

        // Camera
        Camera cam;
        public Matrix4 viewMatrix;
        public Matrix4 projectionMatrix;
        public Matrix4 viewProjectionMatrix;

        // Frame Buffers and render objects
        FBOInfo fboScene;
        FBOInfo fboMerge;
        PostProcess hybridRenderQuad;

        // public Game(int width = 428, int height = 240, string title = "Voxelized Shard") :
        // public Game(int width = 854, int height = 480, string title = "Voxelized Shard") :
        public Game(int width = 1280, int height = 720, string title = "Voxelized Shard") :
            base(
                GameWindowSettings.Default,
                new NativeWindowSettings()
                {
                    Title = title,
                    ClientSize = new Vector2i(width, height),
                    WindowBorder = WindowBorder.Resizable,
                    StartVisible = false,
                    StartFocused = false,
                    API = ContextAPI.OpenGL,
                    Profile = ContextProfile.Core,
                })
        {
            this.CenterWindow();
            this.Width = width;
            this.Height = height;            
        }

        public virtual void Init() { }

        protected override void OnResize(ResizeEventArgs e)
        {
            Width = e.Width; Height = e.Height;
            GL.Viewport(0, 0, Width, Height);
            projectionMatrix = cam.GetProjectionMatrix();

            fboScene.Resize(Width, Height);
            fboMerge.Resize(Width, Height);
            RaichuObject.FBOInfo.Resize(Width, Height);

            base.OnResize(e);
        }

        protected override void OnLoad()
        {
            Bootystrap.Instance.SoundFMOD.InitFMod();

            cam = Bootystrap.Instance.Camera;
            projectionMatrix = cam.GetProjectionMatrix();

            this.IsVisible = true;
            CursorState = CursorState.Grabbed;

            GL.ClearColor(cam.ClearColor);
            GL.Enable(EnableCap.CullFace);
            GL.CullFace(CullFaceMode.Back);


            // Create raytracing FBO
            RaichuObject.FBOInfo = new FBOInfo(Width, Height);
            fboScene = new FBOInfo(Width, Height);
            fboMerge = new FBOInfo(Width, Height);
            hybridRenderQuad = new PostProcess("hybridRender.frag"); 


            Bootystrap.Instance.AssetManager.getPathRelativeToRoot("");
            Debug.Log(Bootystrap.Instance.AssetManager.getPathRelativeToRoot(""));

            Init();

            base.OnLoad();
        }

        protected override void OnUnload()
        {
            GL.UseProgram(0);

            foreach (GameObject obj in SceneObjects)
                if(obj is RastyObject)
                    (obj as RastyObject).Free();

            fboMerge.Free();
            fboScene.Free();
            RaichuObject.FBOInfo.Free();

            base.OnUnload();
        }

        protected override void OnUpdateFrame(FrameEventArgs args)
        {
            base.OnUpdateFrame(args);
            Bootystrap.Instance.SoundFMOD.UpdateFMod();

            Update((float)args.Time);
        }

        protected virtual void Update(float deltaTime) { }

        protected override void OnRenderFrame(FrameEventArgs args)
        {
            // Prepare matrices
            viewMatrix = cam.GetViewMatrix();
            viewProjectionMatrix = viewMatrix * projectionMatrix;

            ///////////////////////
            // Rasterized  pass  //
            ///////////////////////
            // Draw the rasterized meshes into the scene frame buffer
            GL.BindFramebuffer(FramebufferTarget.Framebuffer, fboScene.framebufferId);
            GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);
            GL.Enable(EnableCap.DepthTest);

            foreach (GameObject obj in SceneObjects)
                if (obj is RastyObject)
                    obj.Render();


            ///////////////////////
            //  Raycasted  pass  //
            ///////////////////////
            // Raycast textures into RaichuObject.FboInfo
            var raichuObjectsPresent = false;
            foreach (GameObject obj in SceneObjects)
            {
                if (obj is RaichuObject)
                {
                    obj.Render();
                    DepthMergeFrameBuffers(RaichuObject.FBOInfo, fboScene, 0);
                    BlitFrameBuffer(0, fboScene.framebufferId);
                    raichuObjectsPresent = true;
                }
            }
            if(!raichuObjectsPresent)
                BlitFrameBuffer(fboScene.framebufferId, 0);

            ///////////////////////
            // Post process pass //
            ///////////////////////
            // Prepare for post process
            if (PostProcesses.Count > 0)
            {
                BlitFrameBuffer(0, fboMerge.framebufferId);
                GL.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
                GL.Clear(ClearBufferMask.DepthBufferBit);
                GL.Disable(EnableCap.DepthTest);
            }

            foreach (var postProcess in PostProcesses)
            {
                GL.Clear(ClearBufferMask.ColorBufferBit);
                GL.ActiveTexture(TextureUnit.Texture0);
                GL.BindTexture(TextureTarget.Texture2D, fboMerge.colorBuffer);

                postProcess.Render();
                BlitFrameBuffer(0, fboMerge.framebufferId);
            }

            this.SwapBuffers();
            base.OnRenderFrame(args);
        }

        private void BlitFrameBuffer(int fbSrc, int fbDest)
        {
            GL.BlitNamedFramebuffer(fbSrc, fbDest,
                0, 0, Width, Height,
                0, 0, Width, Height,
                ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit, BlitFramebufferFilter.Nearest);
        }

        // Merges both fbo1 and fb2 into frame buffer with idx == resultFBidx
        private void DepthMergeFrameBuffers(FBOInfo fbo1, FBOInfo fbo2, int resultFBidx)
        {
            GL.BindFramebuffer(FramebufferTarget.Framebuffer, resultFBidx);
            GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);
            GL.DepthFunc(DepthFunction.Lequal);

            GL.ActiveTexture(TextureUnit.Texture0);
            GL.BindTexture(TextureTarget.Texture2D, fbo1.colorBuffer);
            GL.ActiveTexture(TextureUnit.Texture1);
            GL.BindTexture(TextureTarget.Texture2D, fbo2.colorBuffer);
            GL.ActiveTexture(TextureUnit.Texture2);
            GL.BindTexture(TextureTarget.Texture2D, fbo1.depthBuffer);
            GL.ActiveTexture(TextureUnit.Texture3);
            GL.BindTexture(TextureTarget.Texture2D, fbo2.depthBuffer);

            hybridRenderQuad.Render();
        }        
    }
}
