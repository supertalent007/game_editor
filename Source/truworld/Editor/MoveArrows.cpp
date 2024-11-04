#include "MoveArrows.h"

#include "EditorPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "truworld/GameObjects/TruGameObject.h"

// Constructor
AMoveArrows::AMoveArrows()
{
    // Set this actor to call Tick() every frame.
    PrimaryActorTick.bCanEverTick = true;

    // Create a root scene component
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = Root;

    Forward = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Forward"));
    Forward->SetupAttachment(Root);

    Up = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Up"));
    Up->SetupAttachment(Root);

    Right = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right"));
    Right->SetupAttachment(Root);

    TArray<UStaticMeshComponent*> Arrows = { Forward, Up, Right };
    for (UStaticMeshComponent* Arrow : Arrows)
    {
        Arrow->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Arrow->SetGenerateOverlapEvents(true);
        Arrow->SetNotifyRigidBodyCollision(true);
        Arrow->SetCastShadow(false);
        Arrow->SetCollisionResponseToAllChannels(ECR_Block);

        Arrow->SetRenderCustomDepth(true);
        Arrow->SetCustomDepthStencilValue(1);
        Arrow->bRenderInMainPass = true; 
        Arrow->bRenderInDepthPass = false;
    }

    bIsDragging = false;
    DraggedComponent = nullptr;
}

void AMoveArrows::SetVisibility(bool bVisible)
{
    Forward->SetVisibility(bVisible);
    Up->SetVisibility(bVisible);
    Right->SetVisibility(bVisible);
}

void AMoveArrows::BeginPlay()
{
    Super::BeginPlay();

    TArray<UStaticMeshComponent*> Arrows = { Right, Up, Forward };
    for (UStaticMeshComponent* Arrow : Arrows)
    {
        if (Arrow)
        {
            Arrow->OnBeginCursorOver.AddDynamic(this, &AMoveArrows::OnCursorOver);
            Arrow->OnEndCursorOver.AddDynamic(this, &AMoveArrows::OnCursorEnd);
            Arrow->OnClicked.AddDynamic(this, &AMoveArrows::OnArrowClicked);
            Arrow->OnReleased.AddDynamic(this, &AMoveArrows::OnArrowReleased);
        }
    }
}

void AMoveArrows::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if(ATruGameObject* GameObject = CastChecked<AEditorPlayerController>(GetWorld()->GetFirstPlayerController())->GetSelectedObject())
    {
        if(!bIsDragging)
            SetActorLocation(GameObject->GetActorLocation());
        else
            GameObject->SetActorLocation(GetActorLocation());
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
    if (CameraManager)
    {
        FVector CameraLocation = CameraManager->GetCameraLocation();
        FVector ActorLocation = GetActorLocation();

        float Distance = FVector::Dist(CameraLocation, ActorLocation);

        float ScaleValue = Distance / 200.f;
        FVector NewScale(ScaleValue, ScaleValue, ScaleValue);
        SetActorScale3D(NewScale);
    }

    if (bIsDragging && DraggedComponent)
    {
        if (PlayerController)
        {
            FVector2D MousePosition;
            if (PlayerController->GetMousePosition(MousePosition.X, MousePosition.Y))
            {
                FVector WorldOrigin;
                FVector WorldDirection;
                if (PlayerController->DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldOrigin, WorldDirection))
                {
                    // Normalize directions
                    WorldDirection = WorldDirection.GetSafeNormal();

                    // Compute closest point between two infinite lines
                    FVector Origin1 = DragStartLocation;
                    FVector Dir1 = DragDirection;
                    FVector Origin2 = WorldOrigin;
                    FVector Dir2 = WorldDirection;

                    FVector w0 = Origin1 - Origin2;
                    float a = FVector::DotProduct(Dir1, Dir1);
                    float b = FVector::DotProduct(Dir1, Dir2);
                    float c = FVector::DotProduct(Dir2, Dir2);
                    float d = FVector::DotProduct(Dir1, w0);
                    float e = FVector::DotProduct(Dir2, w0);
                    float D = a * c - b * b;

                    float sc, tc;

                    if (D < KINDA_SMALL_NUMBER)
                    {
                        sc = 0.0f;
                        tc = (b > c ? d / b : e / c);
                    }
                    else
                    {
                        sc = (b * e - c * d) / D;
                        tc = (a * e - b * d) / D;
                    }

                    FVector ClosestPointOnAxis = Origin1 + sc * Dir1;

                    // Update actor location
                    FVector NewLocation = ClosestPointOnAxis + DragOffset;
                    SetActorLocation(NewLocation);
                }
            }
        }
    }
}

void AMoveArrows::StartDragging(UStaticMeshComponent* Component, FVector Direction)
{
    bIsDragging = true;
    DraggedComponent = Component;
    DragDirection = Direction.GetSafeNormal();
    DragStartLocation = GetActorLocation();

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
        return;
    FVector2D MousePosition;
    if (PlayerController->GetMousePosition(MousePosition.X, MousePosition.Y))
    {
        FVector WorldOrigin;
        FVector WorldDirection;
        if (PlayerController->DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldOrigin, WorldDirection))
        {
            // Normalize WorldDirection
            WorldDirection = WorldDirection.GetSafeNormal();

            // Define a reasonable length for the mouse ray (e.g., 10000 units)
            float RayLength = 10000.0f;
            FVector MouseRayEnd = WorldOrigin + WorldDirection * RayLength;

            // Define the drag axis as a segment
            FVector AxisEnd = DragStartLocation + DragDirection * RayLength;

            // Calculate closest points between the drag axis and the mouse ray
            FVector ClosestPointOnAxis, ClosestPointOnMouseRay;
            FMath::SegmentDistToSegment(
                DragStartLocation, AxisEnd,
                WorldOrigin, MouseRayEnd,
                ClosestPointOnAxis, ClosestPointOnMouseRay
            );

            // Store initial point and offset
            InitialDragAxisPoint = ClosestPointOnAxis;
            DragOffset = DragStartLocation - ClosestPointOnAxis;
        }
    }
}

void AMoveArrows::StopDragging()
{
    bIsDragging = false;
    DraggedComponent = nullptr;

    // Check if the cursor is over the highlighted arrow
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    FVector2D MousePosition;
    if (PlayerController->GetMousePosition(MousePosition.X, MousePosition.Y))
    {
        FHitResult HitResult;
        PlayerController->GetHitResultAtScreenPosition(MousePosition, ECollisionChannel::ECC_Visibility, true, HitResult);

        if (HitResult.Component.IsValid())
        {
            UPrimitiveComponent* HoveredComponent = HitResult.Component.Get();
            if (HoveredComponent == HighlightedArrow)
            {
                // The cursor is still over the arrow; keep it highlighted
                return;
            }
        }
    }

    // Cursor is not over the arrow; unhighlight it
    if (HighlightedArrow == Right && RightDynamicMaterial)
    {
        RightDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), RightNormalColor);
    }
    else if (HighlightedArrow == Up && UpDynamicMaterial)
    {
        UpDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), UpNormalColor);
    }
    else if (HighlightedArrow == Forward && ForwardDynamicMaterial)
    {
        ForwardDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), ForwardNormalColor);
    }
    HighlightedArrow = nullptr;
}


void AMoveArrows::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Initialize arrow materials and colors
    if (RightMaterial)
    {
        RightDynamicMaterial = UMaterialInstanceDynamic::Create(RightMaterial, this);
        Right->SetMaterial(0, RightDynamicMaterial);

        // Set the default normal and hover colors
        RightNormalColor = FLinearColor::White;    // Default normal color
        RightHoveredColor = FLinearColor::Red;     // Default hover color

        // Set the initial color for Right material
        RightDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), RightNormalColor);
    }

    if (UpMaterial)
    {
        UpDynamicMaterial = UMaterialInstanceDynamic::Create(UpMaterial, this);
        Up->SetMaterial(0, UpDynamicMaterial);

        // Set the default normal and hover colors
        UpNormalColor = FLinearColor::Blue;        // Default normal color
        UpHoveredColor = FLinearColor::Yellow;     // Default hover color

        // Set the initial color for Up material
        UpDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), UpNormalColor);
    }

    if (ForwardMaterial)
    {
        ForwardDynamicMaterial = UMaterialInstanceDynamic::Create(ForwardMaterial, this);
        Forward->SetMaterial(0, ForwardDynamicMaterial);

        // Set the default normal and hover colors
        ForwardNormalColor = FLinearColor::Green;  // Default normal color
        ForwardHoveredColor = FLinearColor::White; // Default hover color

        // Set the initial color for Forward material
        ForwardDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), ForwardNormalColor);
    }
}

void AMoveArrows::OnCursorOver(UPrimitiveComponent* TouchedComponent)
{
    if (bIsDragging)
    {
        return;
    }

    if (TouchedComponent == Right && RightDynamicMaterial)
    {
        RightDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), RightHoveredColor);
        HighlightedArrow = Right;
    }
    else if (TouchedComponent == Up && UpDynamicMaterial)
    {
        UpDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), UpHoveredColor);
        HighlightedArrow = Up;
    }
    else if (TouchedComponent == Forward && ForwardDynamicMaterial)
    {
        ForwardDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), ForwardHoveredColor);
        HighlightedArrow = Forward;
    }
}

void AMoveArrows::OnCursorEnd(UPrimitiveComponent* TouchedComponent)
{
    if (bIsDragging)
    {
        return;
    }

    if (TouchedComponent == HighlightedArrow)
    {
        if (HighlightedArrow == Right && RightDynamicMaterial)
        {
            RightDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), RightNormalColor);
        }
        else if (HighlightedArrow == Up && UpDynamicMaterial)
        {
            UpDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), UpNormalColor);
        }
        else if (HighlightedArrow == Forward && ForwardDynamicMaterial)
        {
            ForwardDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), ForwardNormalColor);
        }
        HighlightedArrow = nullptr;
    }
}

void AMoveArrows::OnArrowClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    // Start dragging the arrow
    if (TouchedComponent == Right)
    {
        StartDragging(Right, GetActorRightVector());
    }
    else if (TouchedComponent == Up)
    {
        StartDragging(Up, GetActorUpVector());
    }
    else if (TouchedComponent == Forward)
    {
        StartDragging(Forward, GetActorForwardVector());
    }

    // Highlight the dragged arrow
    if (TouchedComponent == Right && RightDynamicMaterial)
    {
        RightDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), RightHoveredColor);
        HighlightedArrow = Right;
    }
    else if (TouchedComponent == Up && UpDynamicMaterial)
    {
        UpDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), UpHoveredColor);
        HighlightedArrow = Up;
    }
    else if (TouchedComponent == Forward && ForwardDynamicMaterial)
    {
        ForwardDynamicMaterial->SetVectorParameterValue(TEXT("ArrowColor"), ForwardHoveredColor);
        HighlightedArrow = Forward;
    }
}

void AMoveArrows::OnArrowReleased(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    StopDragging();
}
