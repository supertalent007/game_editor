#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MoveArrows.generated.h"


UENUM()
enum class EArrowMode : uint8
{
    Move,
    Scale,
    Rotate
};

UCLASS()
class TRUWORLD_API AMoveArrows : public AActor
{
    GENERATED_BODY()

public:    
    // Constructor
    AMoveArrows();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    EArrowMode Mode = EArrowMode::Move;
public:    
    virtual void Tick(float DeltaTime) override;

    UFUNCTION() void OnCursorOver(UPrimitiveComponent* TouchedComponent);
    UFUNCTION() void OnCursorEnd(UPrimitiveComponent* TouchedComponent);
    UFUNCTION() void OnArrowClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
    UFUNCTION() void OnArrowReleased(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

    virtual void OnConstruction(const FTransform& Transform) override;

    UFUNCTION(BlueprintCallable) bool IsDragging() const { return bIsDragging; }
    
    void StopDragging();

    UStaticMeshComponent* GetForwardArrow() const { return Forward; }
    UStaticMeshComponent* GetUpArrow() const { return Up; }
    UStaticMeshComponent* GetRightArrow() const { return Right; }

    void SetVisibility(bool bVisible);
private:
    // Root component
    UPROPERTY(VisibleAnywhere)
    USceneComponent* Root;

    // Components
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Forward;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Up;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Right;

    // Materials that can be set in the Editor
    UPROPERTY(EditAnywhere, Category = "Materials")
    UMaterialInterface* RightMaterial;

    UPROPERTY(EditAnywhere, Category = "Materials")
    UMaterialInterface* UpMaterial;

    UPROPERTY(EditAnywhere, Category = "Materials")
    UMaterialInterface* ForwardMaterial;

    UStaticMeshComponent* HighlightedArrow = nullptr;

    // Dynamic Material Instances
    UMaterialInstanceDynamic* RightDynamicMaterial;
    UMaterialInstanceDynamic* ForwardDynamicMaterial;
    UMaterialInstanceDynamic* UpDynamicMaterial;

    // Hover and normal colors for Right, Up, and Forward
    FLinearColor RightNormalColor;
    FLinearColor RightHoveredColor;

    FLinearColor UpNormalColor;
    FLinearColor UpHoveredColor;

    FLinearColor ForwardNormalColor;
    FLinearColor ForwardHoveredColor;

    // Dragging variables
    bool bIsDragging;
    UStaticMeshComponent* DraggedComponent;
    FVector DragDirection;
    FVector DragStartLocation;
    FVector DragOffset;
    FVector InitialDragAxisPoint;
    FVector MouseStartWorldLocation;
    FPlane DragPlane;

    void StartDragging(UStaticMeshComponent* Component, FVector Direction);
};
