#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EditorPlayerController.generated.h"

class AMoveArrows;
class ATruGameObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectSelected, ATruGameObject*, SelectedObject);

UCLASS()
class AEditorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AEditorPlayerController();

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	void OnCopyPressed();

	UPROPERTY(EditAnywhere) TSubclassOf<AMoveArrows> MoveArrowsClass;
	UPROPERTY(EditAnywhere) TSubclassOf<class UEditorUI> EditorUIClass;
	UPROPERTY() TObjectPtr<UEditorUI> EditorUI;
	UFUNCTION(BlueprintCallable) ATruGameObject* GetSelectedObject() const;
	UFUNCTION(BlueprintCallable) AMoveArrows* GetArrows() const;

	void OnGameObjectsRefreshed();

	void SetSelected(ATruGameObject* GameObject);
	bool DragObject();
	
	// New Copy and Paste methods
	UFUNCTION(BlueprintCallable) void CopyObject();
	UFUNCTION(BlueprintCallable) void PasteObject();
	FString GenerateUniqueName(const FString& BaseName);

	UPROPERTY(BlueprintAssignable, Category = "Selection")
	FOnObjectSelected OnObjectSelected;
private:
	bool bIsDraggingObject;
	ATruGameObject* DraggedObject;
	ATruGameObject* CopiedObject;  // Holds the copied actor

	UPROPERTY() AMoveArrows* Arrows;
	UPROPERTY() ATruGameObject* CurrentSelected;
	UPROPERTY() UPrimitiveComponent* CurrentHoveredComponent = nullptr;

	// Mouse input flags
	bool bIsMouseDown;
	bool bIsDragging;

	void OnPastePressed();
	// Input handlers
	void OnLeftMouseDown();
	void OnLeftMouseUp();

	bool bCanSpawn;
	void DragginSpawn() { bCanSpawn = true;}
	void DragginDespawn() { bCanSpawn = false; }
};

FORCEINLINE ATruGameObject* AEditorPlayerController::GetSelectedObject() const
{
	return CurrentSelected;
}

FORCEINLINE AMoveArrows* AEditorPlayerController::GetArrows() const
{
	return Arrows;
}
