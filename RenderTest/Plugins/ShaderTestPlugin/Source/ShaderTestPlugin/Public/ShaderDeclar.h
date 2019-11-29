#pragma once
#include "CoreMinimal.h"
#include "RenderCore/Public/RenderResource.h"
#include "Core/Public/Containers/DynamicRHIResourceArray.h"

struct FMyTextureVertex
{
	FVector4 Position;
	FVector2D UV;
};

class FMyTextureVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	FVertexBufferRHIRef VertexBufferRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FMyTextureVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyTextureVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyTextureVertex, UV), VET_Float2, 1, Stride));
		//VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);

		TResourceArray<FMyTextureVertex> Vertices;
		Vertices.AddUninitialized(4);
		Vertices[0].Position = FVector4(-1.0f, 1.0f, 0, 1.0f);
		Vertices[1].Position = FVector4(1.0f, 1.0f, 0, 1.0f);
		Vertices[2].Position = FVector4(-1.0f, -1.0f, 0, 1.0f);
		Vertices[3].Position = FVector4(1.0f, -1.0f, 0, 1.0f);
		Vertices[0].UV = FVector2D(0, 0);
		Vertices[1].UV = FVector2D(1, 0);
		Vertices[2].UV = FVector2D(0, 1);
		Vertices[3].UV = FVector2D(1, 1);
		FRHIResourceCreateInfo CreateInfo(&Vertices);
		VertexBufferRHI = RHICreateVertexBuffer(sizeof(FMyTextureVertex) * 4, BUF_Static, CreateInfo);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI->Release();
	}
};
