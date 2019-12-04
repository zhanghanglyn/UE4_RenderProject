#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Classes/Kismet/BlueprintFunctionLibrary.h"
#include "MyShaderTest.generated.h"

//���struct�������ⲿ�������õģ� Ҫ��shader��ʹ�õ�uniform�����ڴ����и��ݺ�����̬���ɵģ�
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

	/* ��Texture2Dд������ */
	UFUNCTION(BlueprintCallable, Category = "TextureTestPlugin", meta = (WorldContext = "WorldContextObject"))
		static void WriteTexture(UTexture* MyTexture, AActor* selfref);

	/* ʹ��ComputeShade���㲢�ҽ������� */
	UFUNCTION(BlueprintCallable, Category = "TextureTestPlugin", meta = (WorldContext = "WorldContextObject"))
	static void UseTestComputeShader(class UTextureRenderTarget2D* OutputRenderTarget, AActor* selfref , FString SavePathName);
};
