#include "ApplicationClass.h"

ApplicationClass::ApplicationClass()
{
    //Safety setting to 0/nullptr;
    m_Direct3D = 0;
    m_Camera = 0;
    m_Model = 0;
	m_TextureShader = 0;
	m_Lights = 0;
	m_Bitmap = 0;
	m_TextureNoLightingShader = 0;
}

ApplicationClass::ApplicationClass(const ApplicationClass&)
{
}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Initalize(int screenWidth, int screenHeight, HWND a_WindowHandle)
{
	char textureFilename[128];
	char modelFileName[128];
    bool result;
    m_Direct3D = new Direct3DClass;

    result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, a_WindowHandle, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result)
    {
        MessageBox(a_WindowHandle, L"Could not initialize Direct3D", L"Error", MB_OK);
        return false;
    }

    //create camera
    m_Camera = new CameraClass;
    //set initial position of camera
    m_Camera->SetPosition(0.0f,5.0f,-12.0f);
	m_Camera->SetRotation(25, 0 ,0);

    //create a new model
    m_Model = new ModelClass;

	strcpy_s(textureFilename, "./data/stone01.tga");
	strcpy_s(modelFileName, "./data/plane.txt");
	
    result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), textureFilename, modelFileName);
    if (!result)
    {
        MessageBox(a_WindowHandle, L"Could not initialize model object", L"Error", MB_OK);
        return false;
    }
	
    //create and init the color shader;
	//m_ColorShader = new ColorShaderClass;
    //result = m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd);

	m_TextureShader = new TextureShaderClass;
	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), a_WindowHandle);

    if (!result)
    {
        MessageBox(a_WindowHandle, L"Could not initialize texture shader object", L"Error", MB_OK);
        return false;
    }

	m_TextureNoLightingShader = new TextureNoLightingShaderClass;
	result = m_TextureNoLightingShader->Initialize(m_Direct3D->GetDevice(), a_WindowHandle);
	
    if (!result)
    {
	    MessageBox(a_WindowHandle, L"Could not initialize texture shader object", L"Error", MB_OK);
    	return false;
    }
	
	m_numLights = 4;
	
	m_Lights = new LightClass[m_numLights];

	m_Lights[0].m_DiffuseColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);  // Red
    m_Lights[0].m_Position = XMFLOAT4(-3.0f, 1.0f, 3.0f, 1.0f);

    m_Lights[1].m_DiffuseColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);  // Green
    m_Lights[1].m_Position = XMFLOAT4(3.0f, 1.0f, 3.0f, 1.0f);

    m_Lights[2].m_DiffuseColor = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);  // Blue
    m_Lights[2].m_Position = XMFLOAT4(-3.0f, 1.0f, -3.0f, 1.0f);

    m_Lights[3].m_DiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  // White
    m_Lights[3].m_Position = XMFLOAT4(3.0f, 1.0f, -3.0f, 1.0f);

	m_Bitmap = new BitmapClass;
	result = m_Bitmap->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, textureFilename, 50, 50);
	
	if (!result)
		return false;
	
    return true;
}

void ApplicationClass::Shutdown()
{
    if (m_Direct3D)
    {
        m_Direct3D->Shutdown();
        delete m_Direct3D;
        m_Direct3D = 0;
    }
	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
		m_Bitmap = 0;
	}

    if (m_Lights)
    {
	    delete [] m_Lights;
    	m_Lights = 0;
    }
	
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}
	if (m_TextureNoLightingShader)
	{
		m_TextureNoLightingShader->Shutdown();
		delete m_TextureNoLightingShader;
		m_TextureNoLightingShader = 0;
	}
	
	// Release the model object.
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}
    return;
}

bool ApplicationClass::Frame()
{
	static float rotation = 0.0f;
    bool result;

	//(no damn clue what this number is supposed to be).
	rotation -= 0.0174532825f * 0.1f;
	
	//Render Scene
    result = Render(rotation);
    
    if (!result)
    {
        return false;
    }
    
    return true;
}

bool ApplicationClass::Render(float a_Rotation)
{
	MatrixBufferType matrixBuffer;
	LightPositionBufferType lightPositionBuffer;
	LightColorBufferType lightColorBuffer;
    bool result;
	
    //clear buffers to begin the scene
    m_Direct3D->BeginScene(0.0f,0.0f,0.0f,1.0f);

    //update cameras view matrix
    m_Camera->Render();

    //get all the matrices
	m_Direct3D->GetWorldMatrix(matrixBuffer.world);
	m_Camera->GetViewMatrix(matrixBuffer.view);
	m_Direct3D->GetProjectionMatrix(matrixBuffer.projection);

    //XMMATRIX rotateMatrix, translateMatrix;
	//rotateMatrix = XMMatrixRotationY(a_Rotation);
	//translateMatrix = XMMatrixTranslation(-2.0f, 0.0f, 0.0f);
	//matrixBuffer.world = XMMatrixMultiply(rotateMatrix, translateMatrix);


	for (int i = 0; i < m_numLights; i++)
	{
		lightPositionBuffer.lightPosition[i] = m_Lights[i].m_Position; 
		lightColorBuffer.lightDiffuse[i] = m_Lights[i].m_DiffuseColor; 
	}
	
	//put the model vertex and index buffers into the graphics pipeline to prepare them to be drawn
	m_Model->Render(m_Direct3D->GetDeviceContext());
	
	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(),
									 m_Model->GetIndexCount(),
									 m_Model->GetTexture(),
									 matrixBuffer,
									 lightPositionBuffer,
									 lightColorBuffer);
	if (!result)
	{
		return false;
	}

	//sets location
	//m_Bitmap->SetRenderLocation(50, 100);
	
	m_Direct3D->TurnZBufferOff();

	//m_Bitmap->UpdateHeight(400);
	result = m_Bitmap->Render(m_Direct3D->GetDeviceContext());
	if (!result)
	{
		return false;
	}
	m_Direct3D->GetOrthoMatrix(matrixBuffer.projection);
	result = m_TextureNoLightingShader->Render(m_Direct3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), matrixBuffer.world, matrixBuffer.view, matrixBuffer.projection, m_Bitmap->GetTexture());
	if (!result)
	{
		return false;
	}
	
	m_Direct3D->TurnZBufferOn();
    m_Direct3D->EndScene();
    return true;
}