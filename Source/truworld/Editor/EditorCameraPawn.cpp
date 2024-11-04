// EditorCameraPawn.cpp

#include "EditorCameraPawn.h"

#include "EditorPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MoveArrows.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"


AEditorCameraPawn::AEditorCameraPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and set up the sphere component
    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    RootComponent = SphereComponent;

    // Create and attach the camera component
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(RootComponent);
    CameraComponent->bConstrainAspectRatio = false;

    // Create and configure the movement component
    FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
    FloatingPawnMovement->UpdatedComponent = RootComponent;

    SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
    SceneCaptureComponent->SetupAttachment(CameraComponent);
    SceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = true;
    bIsCameraControlEnabled = false;

    RotationSpeed = 100.0f;
}
 
void AEditorCameraPawn::BeginPlay()
{
    Super::BeginPlay();

    // Get the player controller
    if (AEditorPlayerController* PC = Cast<AEditorPlayerController>(GetController()))
    {
        // Show the mouse cursor
        PC->bShowMouseCursor = true;

        SceneCaptureComponent->ShowOnlyActors.Add(PC->GetArrows());

        // Set input mode to Game and UI
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(InputMode);

        // Add Input Mapping Context to the Enhanced Input Subsystem
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    if (GEngine && GEngine->GameViewport)
    {
        // Calculate aspect ratio from camera's field of view
        FIntPoint ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
        int32 Width = ViewportSize.X;
        int32 Height = ViewportSize.Y;
        CameraComponent->AspectRatio = Width / (float)Height;

        // Create render target with camera's aspect ratio
        RenderTarget = NewObject<UTextureRenderTarget2D>(this);
        RenderTarget->InitAutoFormat(Width, Height);

        // Assign render target to scene capture component
        SceneCaptureComponent->TextureTarget = RenderTarget;

        FPostProcessSettings& PostProcessSettings = CameraComponent->PostProcessSettings;
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetTextureParameterValue(FName("ArrowChannel"), RenderTarget);
            PostProcessSettings.WeightedBlendables.Array.Add({1.f, DynamicMaterial});
        }

        // Bind to viewport resize delegate
        GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &AEditorCameraPawn::OnViewportResized);
    }

    SceneCaptureComponent->FOVAngle = CameraComponent->FieldOfView;
    bIsCameraControlEnabled = false;
}

void AEditorCameraPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->Viewport->ViewportResizedEvent.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

void AEditorCameraPawn::OnViewportResized(FViewport* Viewport, uint32 e)
{
    if (RenderTarget && Viewport)
    {
        FIntPoint NewSize = Viewport->GetSizeXY();

        int32 NewWidth = NewSize.X;
        int32 NewHeight = NewSize.Y;
        CameraComponent->AspectRatio = NewWidth / (float)NewHeight;

        if (NewWidth > 0 && NewHeight > 0)
        {
            RenderTarget->ResizeTarget(NewWidth, NewHeight);
        }
    }
}

void AEditorCameraPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AEditorCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Ensure that the Enhanced Input Component is used
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind movement actions
        EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AEditorCameraPawn::MoveForward);
        EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AEditorCameraPawn::MoveRight);
        EnhancedInputComponent->BindAction(MoveUpAction, ETriggerEvent::Triggered, this, &AEditorCameraPawn::MoveUp);

        // Bind look actions
        EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AEditorCameraPawn::Turn);
        EnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &AEditorCameraPawn::LookUp);

        // Bind mouse actions
        EnhancedInputComponent->BindAction(LeftMouseButtonAction, ETriggerEvent::Started, this, &AEditorCameraPawn::OnLeftMouseButtonPressed);
        EnhancedInputComponent->BindAction(LeftMouseButtonAction, ETriggerEvent::Completed, this, &AEditorCameraPawn::OnLeftMouseButtonReleased);


        EnhancedInputComponent->BindAction(RightMouseButtonAction, ETriggerEvent::Started, this, &AEditorCameraPawn::OnRightMouseButtonPressed);
        EnhancedInputComponent->BindAction(RightMouseButtonAction, ETriggerEvent::Completed, this, &AEditorCameraPawn::OnRightMouseButtonReleased);
    }
}

void AEditorCameraPawn::OnRightMouseButtonPressed(const FInputActionValue& Value)
{
    bIsCameraControlEnabled = true;

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        // Set input mode to Game Only to capture mouse input
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);

        // Hide the mouse cursor
        PC->bShowMouseCursor = false;
    }
}

void AEditorCameraPawn::OnRightMouseButtonReleased(const FInputActionValue& Value)
{
    bIsCameraControlEnabled = false;

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(InputMode);

        PC->bShowMouseCursor = true;
    }
}

void AEditorCameraPawn::OnLeftMouseButtonPressed(const FInputActionValue& Value)
{

}

void AEditorCameraPawn::OnLeftMouseButtonReleased(const FInputActionValue& Value)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundMoveArrows;
        UGameplayStatics::GetAllActorsOfClass(World, AMoveArrows::StaticClass(), FoundMoveArrows);
        for (AActor* Actor : FoundMoveArrows)
        {
            if (AMoveArrows* MoveArrow = Cast<AMoveArrows>(Actor))
            {
                MoveArrow->StopDragging();
            }
        }
    }
}

void AEditorCameraPawn::ExitCameraControl()
{
    if (bIsCameraControlEnabled)
    {
        ToggleCameraControl(); // Reuse the toggle function
    }
}

void AEditorCameraPawn::ToggleCameraControl()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        bIsCameraControlEnabled = !bIsCameraControlEnabled;

        if (bIsCameraControlEnabled)
        {
            // Hide mouse cursor
            PC->bShowMouseCursor = false;

            // Set input mode to Game Only
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
        }
        else
        {
            // Show mouse cursor
            PC->bShowMouseCursor = true;

            // Set input mode to Game and UI
            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
        }
    }
}

void AEditorCameraPawn::MoveForward(const FInputActionValue& Value)
{
    if (bIsCameraControlEnabled)
    {
        float AxisValue = Value.Get<float>();
        if (AxisValue != 0.0f)
        {
            const FVector Direction = CameraComponent->GetForwardVector();
            AddMovementInput(Direction, AxisValue);
        }
    }
}

void AEditorCameraPawn::MoveRight(const FInputActionValue& Value)
{
    if (bIsCameraControlEnabled)
    {
        float AxisValue = Value.Get<float>();
        if (AxisValue != 0.0f)
        {
            const FVector Direction = CameraComponent->GetRightVector();
            AddMovementInput(Direction, AxisValue);
        }
    }
}

void AEditorCameraPawn::MoveUp(const FInputActionValue& Value)
{
    if (bIsCameraControlEnabled)
    {
        float AxisValue = Value.Get<float>();
        if (AxisValue != 0.0f)
        {
            const FVector Direction = CameraComponent->GetUpVector();
            AddMovementInput(Direction, AxisValue);
        }
    }
}

void AEditorCameraPawn::Turn(const FInputActionValue& Value)
{
    if (bIsCameraControlEnabled)
    {
        float AxisValue = Value.Get<float>();
        if (AxisValue != 0.0f)
        {
            AddControllerYawInput(AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds());
        }
    }
}

void AEditorCameraPawn::LookUp(const FInputActionValue& Value)
{
    if (bIsCameraControlEnabled)
    {
        float AxisValue = Value.Get<float>();
        if (AxisValue != 0.0f)
        {
            AddControllerPitchInput(-AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds());
        }
    }
}

