#include "EditorPlayerController.h"

#include "EngineUtils.h"
#include "MoveArrows.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "truworld/GameObjects/TruGameObject.h"
#include "Widgets/EditorUI.h"

AEditorPlayerController::AEditorPlayerController()
{
    bEnableMouseOverEvents = true;
    bEnableClickEvents = true;
    bShowMouseCursor = true;

    CurrentHoveredComponent = nullptr;
    bIsDragging = false;
    bIsMouseDown = false;
    bIsDraggingObject = false;
    DraggedObject = nullptr;
    CopiedObject = nullptr;
}

void AEditorPlayerController::BeginPlay()
{
    Super::BeginPlay();

    EditorUI = CreateWidget<UEditorUI>(this, EditorUIClass);
    EditorUI->AddToViewport();
    
    CurrentSelected = GetWorld()->SpawnActor<ATruGameObject>();
    CurrentSelected->SetActorLocation(GetPawn()->GetActorLocation());

    Arrows = GetWorld()->SpawnActor<AMoveArrows>(MoveArrowsClass);
}

void AEditorPlayerController::OnGameObjectsRefreshed()
{
    if(EditorUI)
        EditorUI->Refresh();
}

void AEditorPlayerController::SetSelected(ATruGameObject* GameObject)
{
    Arrows->SetVisibility(GameObject != nullptr);
    CurrentSelected = GameObject;

    OnObjectSelected.Broadcast(GameObject);
    if (EditorUI)
    {
        EditorUI->OnSelectedObject(GameObject);
    }
}


bool AEditorPlayerController::DragObject()
{
    if (bIsDraggingObject && DraggedObject)
    {
        FVector WorldOrigin;
        FVector WorldDirection;
        FVector2D MousePosition;

        if (GetMousePosition(MousePosition.X, MousePosition.Y))
        {
            if (DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldOrigin, WorldDirection))
            {
                FVector NewLocation = WorldOrigin + WorldDirection * 10000.0f;
                FVector StartLocation = WorldOrigin;
            
                FHitResult HitResult;
                FCollisionQueryParams CollisionParams;
                CollisionParams.AddIgnoredActor(DraggedObject);
                CollisionParams.AddIgnoredActor(Arrows);
            
                if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, NewLocation, ECC_Visibility, CollisionParams))
                {
                    NewLocation = HitResult.Location;
                }
                
                DraggedObject->SetActorLocation(NewLocation);
            }
        }
        return true;
    }
    return false;
}

void AEditorPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    if (DragObject())
        return;
    
    FVector2D MousePosition;
    if (!GetMousePosition(MousePosition.X, MousePosition.Y))
    {
        return;
    }

    FVector WorldOrigin;
    FVector WorldDirection;

    if (!DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldOrigin, WorldDirection))
    {
        return;
    }

    FVector Start = WorldOrigin;
    FVector End = Start + WorldDirection * 10000.0f;

    FHitResult HitResult;
    bool bHit = false;
    UPrimitiveComponent* HitComponent = nullptr;

    if (Arrows)
    {
        TArray<UPrimitiveComponent*> ComponentsToTrace = { Arrows->GetForwardArrow(), Arrows->GetUpArrow(), Arrows->GetRightArrow() };

        FCollisionQueryParams TraceParams(FName(TEXT("ArrowsTrace")), true, this);
        TraceParams.bReturnPhysicalMaterial = false;
        TraceParams.bTraceComplex = true;
        TraceParams.AddIgnoredActor(GetPawn()); // Ignore the player pawn

        for (UPrimitiveComponent* Component : ComponentsToTrace)
        {
            if (Component)
            {
                FHitResult ComponentHitResult;
                bHit = Component->LineTraceComponent(ComponentHitResult, Start, End, TraceParams);
                if (bHit)
                {
                    HitResult = ComponentHitResult;
                    HitComponent = Component;
                    break;
                }
            }
        }
    }

    // Handle cursor over and end events
    if (CurrentHoveredComponent != HitComponent)
    {
        if (CurrentHoveredComponent)
        {
            Arrows->OnCursorEnd(CurrentHoveredComponent);
        }
        if (HitComponent)
        {
            Arrows->OnCursorOver(HitComponent);
        }
        CurrentHoveredComponent = HitComponent;
    }

    // Handle dragging logic
    if (bIsMouseDown)
    {
        if (HitComponent && !bIsDragging)
        {
            Arrows->OnArrowClicked(HitComponent, EKeys::LeftMouseButton);
            bIsDragging = true;
        }
    }
    else
    {
        if (bIsDragging)
        {
            Arrows->OnArrowReleased(CurrentHoveredComponent, EKeys::LeftMouseButton);
            bIsDragging = false;
        }
    }
}


void AEditorPlayerController::CopyObject()
{
    if (CurrentSelected)
    {
        CopiedObject = CurrentSelected; // Store the selected object
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Object Copied!"));
    }
}

void AEditorPlayerController::PasteObject()
{
    if (CopiedObject)
    {
        FTransform CopyTransform = CopiedObject->GetActorTransform(); // Get transform of copied actor
        FString NewName = GenerateUniqueName(CopiedObject->GetName());

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetPawn();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
        SpawnParams.Name = FName(*NewName); // Use the generated unique name

        ATruGameObject* PastedObject = GetWorld()->SpawnActor<ATruGameObject>(CopiedObject->GetClass(), CopyTransform, SpawnParams);

        if (PastedObject)
        {
            SetSelected(PastedObject); // Set the pasted object as selected
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Object Pasted with name: %s"), *NewName));
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No Object to Paste!"));
    }
}

FString AEditorPlayerController::GenerateUniqueName(const FString& BaseName)
{
    int32 Suffix = 0;
    FString NewName = BaseName;

    bool bNameExists = true;
    while (bNameExists)
    {
        bNameExists = false;
        NewName = FString::Printf(TEXT("%s (%d)"), *BaseName, Suffix++);

        for (TActorIterator<ATruGameObject> It(GetWorld()); It; ++It)
        {
            if (It->GetName() == NewName)
            {
                bNameExists = true;
                break;
            }
        }
    }

    return NewName;
}

void AEditorPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AEditorPlayerController::OnLeftMouseDown);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AEditorPlayerController::OnLeftMouseUp);

    InputComponent->BindKey(EKeys::C, IE_Pressed, this, &AEditorPlayerController::DragginSpawn);
    InputComponent->BindKey(EKeys::C, IE_Released, this, &AEditorPlayerController::DragginDespawn);

    InputComponent->BindKey(EKeys::C, IE_Pressed, this, &AEditorPlayerController::OnCopyPressed);
    InputComponent->BindKey(EKeys::V, IE_Pressed, this, &AEditorPlayerController::OnPastePressed);
}

void AEditorPlayerController::OnCopyPressed()
{
    if (IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl))
    {
        CopyObject();
    }
}

void AEditorPlayerController::OnPastePressed()
{
    if (IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl))
    {
        PasteObject();
    }
}
void AEditorPlayerController::OnLeftMouseDown()
{
    bIsMouseDown = true;
    
    if (!bIsDraggingObject && bCanSpawn)
    {
        FVector WorldOrigin;
        FVector WorldDirection;
        FVector2D MousePosition;

        if (GetMousePosition(MousePosition.X, MousePosition.Y))
        {
            if (DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldOrigin, WorldDirection))
            {
                FVector TtSpawnLocation = WorldOrigin + WorldDirection * 1000.0f;
                
                FHitResult HitResult;
                FVector TraceEnd = WorldOrigin + WorldDirection * 1000.0f;
                
                FCollisionQueryParams CollisionParams;
                CollisionParams.AddIgnoredActor(this->GetPawn());
                
                bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, WorldOrigin, TraceEnd, ECC_Visibility, CollisionParams);
                
                if (bHit) 
                {
                    TtSpawnLocation = HitResult.Location;
                }
                
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = this;
                SpawnParams.Instigator = GetPawn();
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                DraggedObject = GetWorld()->SpawnActor<ATruGameObject>(ATruGameObject::StaticClass(), TtSpawnLocation, FRotator::ZeroRotator, SpawnParams);

                if (DraggedObject)
                {
                    SetSelected(DraggedObject);
                    bIsDraggingObject = true;
                }
            }
        }
        return;
    }

    FVector WorldOrigin;
    FVector WorldDirection;
    FVector2D MousePosition;
    if (GetMousePosition(MousePosition.X, MousePosition.Y))
    {
        if (DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldOrigin, WorldDirection))
        {
            FVector NewLocation = WorldOrigin + WorldDirection * 10000.0f;
            FVector StartLocation = WorldOrigin;
            
            FHitResult HitResult;
            FCollisionQueryParams CollisionParams;
            if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, NewLocation, ECC_Visibility, CollisionParams))
            {
                ATruGameObject* TruGameObject = Cast<ATruGameObject>(HitResult.GetActor());
                if(TruGameObject && HitResult.GetActor() != Arrows)
                    SetSelected(TruGameObject);
            }
            else
            {
                SetSelected(nullptr);
            }
        }
    }
}

void AEditorPlayerController::OnLeftMouseUp()
{
    bIsMouseDown = false;
    bIsDraggingObject = false;
    DraggedObject = nullptr;
}