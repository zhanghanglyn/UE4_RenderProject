#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Classes/Kismet/BlueprintFunctionLibrary.h"
#include "MyShaderTest.generated.h"

//这个struct是用来外部进行设置的！ 要在shader中使用的uniform，是在代码中根据宏来动态生成的！
USTRUCT(BlueprintType)
struct FMyColorUniform
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Uniform Color")
		int32 ColorIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Uniform Color")
		FLinearColor ColorOne;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Uniform Color")
		FLinearColor ColorTwo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Uniform Color")
		FLinearColor ColorThree;
};

UCLASS(MinimalAPI , meta = (ScriptName = "TestShaderLibrary"))
class UTestShaderBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable , Category = "ShaderTestPlugin" , meta = (WorldContext = "WorldContextObject"))
	static void DrawTestShaderRenderTarget(class UTextureRenderTarget2D* OutputRenderTarget, AActor* AC,
		FLinearColor MyColor , UTexture* MyTexture , FMyColorUniform ColorUniformBuffer);

	/* 向Texture2D写入内容 */
	UFUNCTION(BlueprintCallable, Category = "TextureTestPlugin", meta = (WorldContext = "WorldContextObject"))
		static void WriteTexture(UTexture* MyTexture, AActor* selfref);

	/* 使用ComputeShade计算并且将结果输出 */
	UFUNCTION(BlueprintCallable, Category = "TextureTestPlugin", meta = (WorldContext = "WorldContextObject"))
	static void UseTestComputeShader(class UTextureRenderTarget2D* OutputRenderTarget, AActor* selfref , FString SavePathName);
};
