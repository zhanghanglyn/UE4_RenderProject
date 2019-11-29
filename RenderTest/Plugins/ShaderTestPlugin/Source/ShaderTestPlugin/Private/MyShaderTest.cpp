#pragma once
#include "MyshaderTest.h"
#include "Classes/Engine/TextureRenderTarget2D.h"  
#include "Classes/Engine/World.h"  
#include "Public/GlobalShader.h"  
#include "Public/PipelineStateCache.h"  
#include "Public/RHIStaticStates.h"  
#include "Public/SceneUtils.h"  
#include "Public/SceneInterface.h"  
#include "Public/ShaderParameterUtils.h"  
#include "Public/Logging/MessageLog.h"  
#include "Public/Internationalization/Internationalization.h"  
#include "Engine/Classes/Engine/Texture.h"
#include "Public/StaticBoundShaderState.h" 
#include "RHI/Public/RHIResources.h"
#include "ShaderDeclar.h"
#include "Engine/Classes/Engine/Texture.h"
#include "RenderCore/Public/UniformBuffer.h"
#include "RHI/Public/RHIResources.h"
#include "MyComputeShader.h"
#include "RHICommandList.h"

//#include "RenderCore/Public/RenderResource.h"

#define LOCTEXT_NAMESPACE "TestShader"

/*BEGIN_UNIFORM_BUFFER_STRUCT(FMyColorUniformStruct, )
UNIFORM_MEMBER(FVector4, ColorOne)
UNIFORM_MEMBER(FVector4, ColorTwo)
UNIFORM_MEMBER(FVector4, ColorThree)
UNIFORM_MEMBER( uint32 , ColorIndex)
END_UNIFORM_BUFFER_STRUCT(FMyColorUniformStruct)
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FMyColorUniformStruct , TEXT( "FMyColorUniform" )) */
//ԭ���ķ�����TM�������ˣ���
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMyColorUniformStruct, )
	SHADER_PARAMETER(FVector4, ColorOne)
	SHADER_PARAMETER(FVector4, ColorTwo)
	SHADER_PARAMETER(FVector4, ColorThree)
	SHADER_PARAMETER(uint32, ColorIndex)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMyColorUniformStruct, "FMyColorUniform");

UTestShaderBlueprintLibrary::UTestShaderBlueprintLibrary(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{ }

//�ڴ˶���һ����MyShader.usf shader���й�������
class FMyShaderTest : public FGlobalShader
{
public:
	FMyShaderTest() {}

	FMyShaderTest(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer)
	{
		SimpleColorVal.Bind(Initializer.ParameterMap, TEXT("SimpleColor"));
		TextureVal.Bind(Initializer.ParameterMap, TEXT("MyTexture"));
		TextureSampler.Bind(Initializer.ParameterMap, TEXT("MyTextureSampler"));
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		//return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM4);  
		//return (*ShouldCompilePermutationRef)(FGlobalShaderPermutationParameters(Platform, PermutationId));
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("TEST_MICRO"), 1);
	}

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << SimpleColorVal<< TextureVal;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FRHICommandListImmediate& RHICmdList, const FLinearColor &MyColor , FTextureRHIParamRef &MyTexture,
		FMyColorUniform &ColorUniform
		)// FRHITexture* MyTexture )
	{
		SetShaderValue(RHICmdList, GetPixelShader(), SimpleColorVal, MyColor);
		SetTextureParameter(RHICmdList, GetPixelShader(), TextureVal, TextureSampler,
			TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), MyTexture);

		FMyColorUniformStruct UniformData;
		UniformData.ColorOne = ColorUniform.ColorOne;
		UniformData.ColorTwo = ColorUniform.ColorTwo;
		UniformData.ColorThree = ColorUniform.ColorThree;
		UniformData.ColorIndex = ColorUniform.ColorIndex;

		SetUniformBufferParameterImmediate(RHICmdList, GetPixelShader(), GetUniformBufferParameter<FMyColorUniformStruct>(), UniformData);
	
	}


	private:
	FShaderParameter SimpleColorVal;
	FShaderResourceParameter TextureVal;
	FShaderResourceParameter TextureSampler;
	
};

class FShaderTestVS : public FMyShaderTest
{
	DECLARE_SHADER_TYPE(FShaderTestVS, Global);

public:
	FShaderTestVS() {};

	FShaderTestVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FMyShaderTest(Initializer)
	{

	}
};

class FShaderTestPS : public FMyShaderTest
{
	DECLARE_SHADER_TYPE(FShaderTestPS, Global);

public:
	FShaderTestPS() {};

	FShaderTestPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FMyShaderTest(Initializer)
	{
		
	}
};

IMPLEMENT_SHADER_TYPE(, FShaderTestVS, TEXT("/Plugin/ShaderTestPlugin/Private/MyShader.usf"), TEXT("MainVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FShaderTestPS, TEXT("/Plugin/ShaderTestPlugin/Private/MyShader.usf"), TEXT("MainPS"), SF_Pixel)

void UTestShaderBlueprintLibrary::DrawTestShaderRenderTarget(UTextureRenderTarget2D* OutputRenderTarget,
	AActor* Ac, FLinearColor MyColor, UTexture* MyTexture , FMyColorUniform ColorUniformBuffer
)
{
	check(IsInGameThread());

	if (!OutputRenderTarget)
	{
		return;
	}

	if (MyTexture == nullptr)
	{
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	UWorld* World = Ac->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
	FName TextureRenderTargetName = OutputRenderTarget->GetFName();
	//FRHITexture* MyTextureRHI = MyTexture->TextureReference.TextureReferenceRHI;
	FTextureRHIParamRef MyTextureRHI = MyTexture->TextureReference.TextureReferenceRHI;

	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel,TextureRenderTargetName ,MyColor , MyTextureRHI , ColorUniformBuffer](FRHICommandListImmediate& RHICmdList)
	{
		DrawTestShaderRenderTarget_RenderThread(RHICmdList, TextureRenderTargetResource, FeatureLevel, TextureRenderTargetName, MyColor , MyTextureRHI , ColorUniformBuffer);
	}
	);

}

//////////����ͼƬ�����
/* ��Texture2Dд������,Ŀǰ�������滻ͼƬ��Դ��������д������ */
#pragma optimize("" , off)
void UTestShaderBlueprintLibrary::WriteTexture(UTexture* MyTexture, AActor* selfref)
{
	check(IsInGameThread());

	if (MyTexture == nullptr || selfref == nullptr)
	{
		return;
	}

	//ѹ�����ã�����MINMAP��
	MyTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	MyTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	MyTexture->SRGB = false;
	MyTexture->UpdateResource();

	FTexturePlatformData* aaa = *MyTexture->GetRunningPlatformData();
	FTexture2DMipMap &MinMap = aaa->Mips[0];
	void* Data = MinMap.BulkData.Lock(LOCK_READ_WRITE);

	int32 textureX = (*(MyTexture->GetRunningPlatformData()))->SizeX;
	int32 textureY = (*(MyTexture->GetRunningPlatformData()))->SizeY;
	TArray<FColor> colors;
	//�ֶ�������� �ô�����ֻ�����һ����ɫ���أ�����ARGB��仹��һЩ
	for (int32 x = 0; x < textureX * textureY; x++)
	{
		//���м仭����
		int32 cur_row = x % textureX;
		int32 max_row = textureY / 2 + 10;
		if (max_row > textureY)
			max_row = textureY -1;
		int32 min_row = textureY / 2 - 10;
		if (min_row < 0)
			min_row = 1;
		if (cur_row <= max_row)
			colors.Add(FColor::Red);
		else
			colors.Add(FColor::Green);
	}

	int32 ColorStride = (int32)(sizeof(uint8) * 4);

	FMemory::Memcpy(
		Data, colors.GetData(), ColorStride * textureX * textureY
	);

	MinMap.BulkData.Unlock();
	//MyTexture->CompressionSettings = OldCompressionSettings;
	//MyTexture->MipGenSettings = OldMipGenSettings;
	//MyTexture->SRGB = OldSRGB;
	MyTexture->UpdateResource();

}
#pragma optimize("" , on)



/* ʹ��ComputeShade���㲢�ҽ������� */
void UTestShaderBlueprintLibrary::UseTestComputeShader(UTextureRenderTarget2D* OutputRenderTarget, AActor* selfref)
{
	check(IsInGameThread());

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	UWorld* World = selfref->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
	FName TextureRenderTargetName = OutputRenderTarget->GetFName();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel , TextureRenderTargetName](FRHICommandListImmediate& RHICmdList)
	{
		UseComputeShader_RenderThread(RHICmdList, TextureRenderTargetResource, FeatureLevel , TextureRenderTargetName);
	}
	);

}


static void DrawTestShaderRenderTarget_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutputRenderTargetResource,
	ERHIFeatureLevel::Type FeatureLevel,
	FName TextureRenderTargetName,
	FLinearColor MyColor,
	//FRHITexture* MyTexture
	FTextureRHIParamRef MyTexture,
	FMyColorUniform ColorUniformBuffer
)
{
	check(IsInRenderingThread());

#if WANTS_DRAW_MESH_EVENTS
	FString EventName;
	TextureRenderTargetName.ToString(EventName);
	/*
	   ��UE4�в���Event��Ҫ��ͨ������SCOPED_DRAW_EVENT��SCOPED_CONDITIONAL_DRAW_EVENT�Ⱥ�������һ��
	   TDrawEvent<TRHICmdList>���󲢵���Start������Ȼ���ڶ�������ʱ������Stop����
	*/
	SCOPED_DRAW_EVENTF(RHICmdList, SceneCapture, TEXT("ShaderTest %s"), *EventName);
#else  
	SCOPED_DRAW_EVENT(RHICmdList, DrawTestShaderRenderTarget_RenderThread);
#endif  

	//������ȾĿ��    //������Ҳ�ϳ���
	/*SetRenderTarget(
		RHICmdList,
		OutputRenderTargetResource->GetRenderTargetTexture(),
		FTextureRHIRef(),
		ESimpleRenderTargetMode::EUninitializedColorAndDepth,
		FExclusiveDepthStencil::DepthNop_StencilNop
	);*/
	//�����´������滻
	FRHITexture2D* RenderTargetTexture = OutputRenderTargetResource->GetRenderTargetTexture();
	RHICmdList.TransitionResource(EResourceTransitionAccess::EWritable, RenderTargetTexture);
	FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::DontLoad_Store, OutputRenderTargetResource->TextureRHI);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawTestShader"));
	{
		FIntPoint DisplacementMapResolution(OutputRenderTargetResource->GetSizeX(), OutputRenderTargetResource->GetSizeY());

		// Update viewport.
		//RHICmdList.SetViewport(0, 0, 0.f,DisplacementMapResolution.X, DisplacementMapResolution.Y, 1.f);

		TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef<FShaderTestVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FShaderTestPS> PixelShader(GlobalShaderMap);

		//����Զ���Shader����ṹ
		FMyTextureVertexDeclaration VertexDeclar;
		VertexDeclar.InitRHI();

		// Set the graphic pipeline state.  
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDeclar.VertexDeclarationRHI;// GetVertexDeclarationFVector4();
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		//RHICmdList.SetViewport(0, 0, 0.f,OutputRenderTargetResource->GetSizeX(), OutputRenderTargetResource->GetSizeY(), 1.f);
		PixelShader->SetParameters(RHICmdList, MyColor, MyTexture, ColorUniformBuffer);

		// Draw grid.
		//uint32 PrimitiveCount = 32 * 16 * 2;
		//RHICmdList.DrawPrimitive(0, PrimitiveCount, 1);
		//�����д�����ᰴ�������εķ�ʽ����
		RHICmdList.SetStreamSource(0, VertexDeclar.VertexBufferRHI, 0);
		RHICmdList.DrawPrimitive(0, 2, 1);
	}
	RHICmdList.EndRenderPass();

}

/* ʹ��ComputeShade���㲢�ҽ������� */
//19.11.29 Ŀǰ�����Ȱ����м����������
static void UseComputeShader_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutputRenderTargetResource,
	ERHIFeatureLevel::Type FeatureLevel,
	FName TextureRenderTargetName
)
{
	check(IsInRenderingThread());

	/*  �ȵ���computeShader�����Ҫ��ʾRenderTexture */
	TShaderMapRef<FMyComputeShader> ComputeShader(GetGlobalShaderMap(FeatureLevel));
	RHICmdList.SetComputeShader(ComputeShader->GetComputeShader());

	int32 SizeX = OutputRenderTargetResource->GetSizeX();
	int32 SizeY = OutputRenderTargetResource->GetSizeY();

	//���������֪����ʲô���UAV

	//FUnorderedAccessViewRHIRef TextureUAV = RHICreateUnorderedAccessView();
	//ComputeShader->BindSurfaces(RHICmdList, TextureUAV);

	/*
	֮���Ե���*�Ǻţ�����ΪTShaderMapRef�����ˣ�
		FORCEINLINE ShaderType* operator*() const
		{
			return Shader;
		}
	*/
	DispatchComputeShader(RHICmdList, *ComputeShader, 32, 32, 1);

}



#undef LOCTEXT_NAMESPACE  