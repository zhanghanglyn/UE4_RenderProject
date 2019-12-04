#pragma once
#include "Public/GlobalShader.h"  
#include "Public/ShaderParameterUtils.h"  
#include "RHI/Public/RHIResources.h"

class FMyComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FMyComputeShader, Global);

public:
	FMyComputeShader() {}

	FMyComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer)
	{
		OutPutTexture.Bind(Initializer.ParameterMap, TEXT("OutputSurface"));
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		//return (*ShouldCompilePermutationRef)(FGlobalShaderPermutationParameters(Platform, PermutationId));
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
	}

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << OutPutTexture;
		return bShaderHasOutdatedParameters;
	}

	void BindSurfaces(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef OutputSurfaceUAV)
	{
		FRHIComputeShader* ComputeShaderRHI = GetComputeShader();
		if (OutPutTexture.IsBound())
			RHICmdList.SetUAVParameter(ComputeShaderRHI, OutPutTexture.GetBaseIndex(), OutputSurfaceUAV);
	}

	void UnbindBuffers(FRHICommandList& RHICmdList)
	{
		FRHIComputeShader* ComputeShaderRHI = GetComputeShader();
		if (OutPutTexture.IsBound())
			RHICmdList.SetUAVParameter(ComputeShaderRHI, OutPutTexture.GetBaseIndex(), FUnorderedAccessViewRHIRef());
	}


	//
	FShaderResourceParameter OutPutTexture;

};

IMPLEMENT_SHADER_TYPE(, FMyComputeShader, TEXT("/Plugin/ShaderTestPlugin/Private/MyShader.usf"), TEXT("MainCS"), SF_Compute)
