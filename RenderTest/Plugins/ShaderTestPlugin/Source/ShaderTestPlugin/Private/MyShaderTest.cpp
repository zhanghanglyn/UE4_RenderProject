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
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Runtime/Core/Public/Misc/Paths.h"
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
//原来的方法又TM不能用了！！
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMyColorUniformStruct, )
	SHADER_PARAMETER(FVector4, ColorOne)
	SHADER_PARAMETER(FVector4, ColorTwo)
	SHADER_PARAMETER(FVector4, ColorThree)
	SHADER_PARAMETER(uint32, ColorIndex)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMyColorUniformStruct, "FMyColorUniform");

UTestShaderBlueprintLibrary::UTestShaderBlueprintLibrary(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{ }

//在此定义一个与MyShader.usf shader进行关联的类
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

	void SetParameters(FRHICommandListImmediate& RHICmdList, const FLinearColor &MyColor , FRHITexture* &MyTexture,
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


static void DrawTestShaderRenderTarget_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutputRenderTargetResource,
	ERHIFeatureLevel::Type FeatureLevel,
	FName TextureRenderTargetName,
	FLinearColor MyColor,
	//FRHITexture* MyTexture
	FRHITexture* MyTexture,
	FMyColorUniform ColorUniformBuffer
)
{
	check(IsInRenderingThread());

#if WANTS_DRAW_MESH_EVENTS
	FString EventName;
	TextureRenderTargetName.ToString(EventName);
	/*
	   在UE4中插入Event主要是通过调用SCOPED_DRAW_EVENT、SCOPED_CONDITIONAL_DRAW_EVENT等宏来构造一个
	   TDrawEvent<TRHICmdList>对象并调用Start方法，然后在对象析构时调用其Stop方法
	*/
	SCOPED_DRAW_EVENTF(RHICmdList, SceneCapture, TEXT("ShaderTest %s"), *EventName);
#else  
	SCOPED_DRAW_EVENT(RHICmdList, DrawTestShaderRenderTarget_RenderThread);
#endif  

	//设置渲染目标    //这他妈也废除了
	/*SetRenderTarget(
		RHICmdList,
		OutputRenderTargetResource->GetRenderTargetTexture(),
		FTextureRHIRef(),
		ESimpleRenderTargetMode::EUninitializedColorAndDepth,
		FExclusiveDepthStencil::DepthNop_StencilNop
	);*/
	//用以下代码来替换
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

		//添加自定义Shader输入结构
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
		//上面的写法，会按照三角形的方式拉伸
		RHICmdList.SetStreamSource(0, VertexDeclar.VertexBufferRHI, 0);
		RHICmdList.DrawPrimitive(0, 2, 1);
	}
	RHICmdList.EndRenderPass();

}

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
	FRHITexture* MyTextureRHI = MyTexture->TextureReference.TextureReferenceRHI;

	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel,TextureRenderTargetName ,MyColor , MyTextureRHI , ColorUniformBuffer](FRHICommandListImmediate& RHICmdList)
	{
		DrawTestShaderRenderTarget_RenderThread(RHICmdList, TextureRenderTargetResource, FeatureLevel, TextureRenderTargetName, MyColor , MyTextureRHI , ColorUniformBuffer);
	}
	);

}

//////////绘制图片等相关
/* 向Texture2D写入内容,目前仅仅是替换图片资源并向其中写入数据 */
#pragma optimize("" , off)
void UTestShaderBlueprintLibrary::WriteTexture(UTexture* MyTexture, AActor* selfref)
{
	check(IsInGameThread());

	if (MyTexture == nullptr || selfref == nullptr)
	{
		return;
	}

	//压缩设置，设置MINMAP等
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
	//手动填充像素 好蠢。。只是填充一个颜色像素，不是ARGB填充还好一些
	for (int32 x = 0; x < textureX * textureY; x++)
	{
		//在中间画个框
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

/* 使用ComputeShade计算并且将结果输出 */
//19.11.29 目前测试先把其中几个像素输出
static void UseComputeShader_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutputRenderTargetResource,
	ERHIFeatureLevel::Type FeatureLevel,
	FName TextureRenderTargetName,
	FString SavePathName
)
{
	check(IsInRenderingThread());

	/*  先调用computeShader计算出要显示RenderTexture */
	TShaderMapRef<FMyComputeShader> ComputeShader(GetGlobalShaderMap(FeatureLevel));
	RHICmdList.SetComputeShader(ComputeShader->GetComputeShader());

	int32 SizeX = OutputRenderTargetResource->GetSizeX();
	int32 SizeY = OutputRenderTargetResource->GetSizeY();

	//创建一个UAV。视图接口指定管道在呈现期间可以访问的资源部分,UAV可以使用FRHITEXTURE来创建，所以先创建一个Texture
	FRHIResourceCreateInfo CreateInfo;
	/*typedef TRefCountPtr<FRHITexture2D> FTexture2DRHIRef; */
	//FTexture2DRHIRef RHITexture = RHICreateTexture2D(SizeX,SizeY, PF_R32_UINT,1,1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
	FTexture2DRHIRef RHITexture = RHICreateTexture2D(SizeX, SizeY, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo); //因为TexCreate_UAV格式说慎用
	FUnorderedAccessViewRHIRef TextureUAV = RHICreateUnorderedAccessView(RHITexture);
	ComputeShader->BindSurfaces(RHICmdList, TextureUAV);

	/*
	之所以调用*星号，是因为TShaderMapRef重载了！
		FORCEINLINE ShaderType* operator*() const
		{
			return Shader;
		}
	*/
	DispatchComputeShader(RHICmdList, *ComputeShader, SizeX/32, SizeY/32, 1);

	ComputeShader->UnbindBuffers(RHICmdList);

	//create a bitmap  
	TArray<FColor> Bitmap;
	uint32 Stride = 0;
	//获取了指向资源地址的指针，取出指针指向的内容即为一个颜色代码，根据位运算获取ARGB的值
	void* Data = RHICmdList.LockTexture2D(RHITexture, 0, EResourceLockMode::RLM_ReadOnly, Stride, false);
	
	char* CharData = (char*)Data;//如果不先转为Char会如何？因为要用步长来进行指针移动，所以我试试转成UINT32呢直接点
	//遍历每一行每一列的颜色代码
	for (uint32 row = 0; row < RHITexture->GetSizeY(); row++)
	{
		//行的指针
		uint32 *RowPtr = (uint32*)CharData;

		for (uint32 column = 0; column < RHITexture->GetSizeX(); column++)
		{
			//当前的颜色地址值
			uint32 ColumnPtr = *RowPtr;

			uint8 R = (ColumnPtr & 0x000000FF);
			uint8 G = (ColumnPtr & 0x0000FF00) >> 8;
			uint8 B = (ColumnPtr & 0x00FF0000) >> 16;
			uint8 A = (ColumnPtr & 0xFF000000) >> 24;

			Bitmap.Add(FColor(R, G, B, A));

			RowPtr++;
		}
		CharData += Stride;
	}

	RHICmdList.UnlockTexture2D(RHITexture, 0, false);

	//颜色我有了，现在试着把颜色输出到一张RenderMap
	if (Bitmap.Num())
	{
		//IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);
		const FString ComputeFileName(FPaths::ProjectContentDir() + SavePathName);

		uint32 ExtendXWithMSAA = Bitmap.Num() / RHITexture->GetSizeY();  //其实我觉得，直接用GetSizeY应该是一样的

		FFileHelper::CreateBitmap(*ComputeFileName, ExtendXWithMSAA, RHITexture->GetSizeY(), Bitmap.GetData());
	}

}


/* 使用ComputeShade计算并且将结果输出 */
void UTestShaderBlueprintLibrary::UseTestComputeShader(UTextureRenderTarget2D* OutputRenderTarget, AActor* selfref , FString SavePathName)
{
	check(IsInGameThread());

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	UWorld* World = selfref->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
	FName TextureRenderTargetName = OutputRenderTarget->GetFName();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel, TextureRenderTargetName , SavePathName](FRHICommandListImmediate& RHICmdList)
	{
		UseComputeShader_RenderThread(RHICmdList, TextureRenderTargetResource, FeatureLevel, TextureRenderTargetName , SavePathName);
	}
	);

}



#undef LOCTEXT_NAMESPACE  