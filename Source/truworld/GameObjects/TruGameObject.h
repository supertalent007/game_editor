// TruGameObject.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TruGameObject.generated.h"

UCLASS()
class TRUWORLD_API ATruGameObject : public AActor
{
	GENERATED_BODY()
    
public:
	ATruGameObject();
	virtual ~ATruGameObject();


	void OnSelected();
	void OnDeselected();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void NotifyEditorPlayerController();

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BoxMesh;
};
