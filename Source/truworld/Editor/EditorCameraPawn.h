// EditorCameraPawn.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EditorCameraPawn.generated.h"

UCLASS()
class TRUWORLD_API AEditorCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AEditorCameraPawn();

protected:
	virtual void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type EndPlayReason);

	// Movement functions
	// Movement functions
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void MoveUp(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void LookUp(const FInputActionValue& Value);

	// Mouse input functions
	void OnLeftMouseButtonPressed(const FInputActionValue& Value);
	void OnLeftMouseButtonReleased(const FInputActionValue& Value);

	void OnRightMouseButtonPressed(const FInputActionValue& Value);
	void OnRightMouseButtonReleased(const FInputActionValue& Value);

	bool bIsCameraControlEnabled;

	// Input Mapping Context
	UPROPERTY(EditAnywhere, Category = "Input")
	UMaterialInterface* PostProcessMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	// Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* TurnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* RightMouseButtonAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LeftMouseButtonAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneCaptureComponent2D* SceneCaptureComponent;

	// Render target
	UPROPERTY(Transient)
	class UTextureRenderTarget2D* RenderTarget;

	void OnViewportResized(FViewport* Viewport, uint32 e);
public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void ExitCameraControl();
	void ToggleCameraControl();

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	class UFloatingPawnMovement* FloatingPawnMovement;

	// Rotation speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float RotationSpeed;
};
