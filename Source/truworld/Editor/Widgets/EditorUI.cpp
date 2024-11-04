#include "EditorUI.h"
#include "Engine/World.h"
#include "truworld/GameObjects/TruGameObject.h"
#include "TruGameObjectWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"

void UEditorUI::Refresh()
{
	UE_LOG(LogTemp, Log, TEXT("Refresh called! ATruGameObject was added or removed."));

	if (!ObjectsInLevel)
	{
		return;
	}

	// Clear existing widgets
	ObjectsInLevel->ClearChildren();

	// Get all ATruGameObject instances in the level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATruGameObject::StaticClass(), FoundActors);

	// Build a parent-to-children map
	TMap<ATruGameObject*, TArray<ATruGameObject*>> ParentToChildrenMap;

	for (AActor* Actor : FoundActors)
	{
		ATruGameObject* GameObject = Cast<ATruGameObject>(Actor);
		if (GameObject)
		{
			ATruGameObject* Parent = Cast<ATruGameObject>(GameObject->GetParentActor());
			ParentToChildrenMap.FindOrAdd(Parent).Add(GameObject);
		}
	}

	// Start with root objects (objects with no parent)
	TArray<ATruGameObject*>* RootObjects = ParentToChildrenMap.Find(nullptr);
	if (RootObjects)
	{
		for (ATruGameObject* RootObject : *RootObjects)
		{
			AddGameObjectWidget(RootObject, 0, ParentToChildrenMap);
		}
	}
}

void UEditorUI::AddGameObjectWidget(ATruGameObject* GameObject, int32 IndentLevel, TMap<ATruGameObject*, TArray<ATruGameObject*>>& ParentToChildrenMap)
{
	if (!GameObject || !ObjectsInLevel)
	{
		return;
	}

	UTruGameObjectWidget* Widget = CreateWidget<UTruGameObjectWidget>(this, GameObjectWidgetClass);
	if (Widget)
	{
		Widget->Setup(GameObject, this);

		// Add the widget to the vertical box
		UVerticalBoxSlot* VerticalBoxSlot = ObjectsInLevel->AddChildToVerticalBox(Widget);
		if (VerticalBoxSlot)
		{
			// Set padding based on indent level
			float LeftPadding = IndentLevel * 20.f;
			VerticalBoxSlot->SetPadding(FMargin(LeftPadding, 0.f, 0.f, 0.f));
		}

		// Recursively add children
		TArray<ATruGameObject*>* Children = ParentToChildrenMap.Find(GameObject);
		if (Children)
		{
			for (ATruGameObject* Child : *Children)
			{
				AddGameObjectWidget(Child, IndentLevel + 1, ParentToChildrenMap);
			}
		}
	}
}

void UEditorUI::OnSelectedObject(ATruGameObject* SelectedGameObject)
{
	// You mentioned you'll implement the rest
	for(UWidget* widget : ObjectsInLevel->GetAllChildren())
	{
		UTruGameObjectWidget* casted_widget = Cast<UTruGameObjectWidget>(widget);
		if(casted_widget)
			casted_widget->UpdateBorderColor();
	}
}

void UEditorUI::NativeConstruct()
{
	Super::NativeConstruct();
	Refresh();
}

void UEditorUI::NativeDestruct()
{
	Super::NativeDestruct();
}