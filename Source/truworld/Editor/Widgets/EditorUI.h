#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EditorUI.generated.h"

UCLASS()
class TRUWORLD_API UEditorUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Refresh();
	void OnSelectedObject(class ATruGameObject* SelectedGameObject);

	class UContextMenuWidget* GetContextWindow() const { return ContextMenuWidget; }
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget)) TObjectPtr<class UContextMenuWidget> ContextMenuWidget;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<class UVerticalBox> ObjectsInLevel;
	UPROPERTY(EditAnywhere) TSubclassOf<class UTruGameObjectWidget> GameObjectWidgetClass;
private:
	FDelegateHandle ActorSpawnedDelegateHandle;

	void AddGameObjectWidget(ATruGameObject* GameObject, int32 IndentLevel, TMap<ATruGameObject*, TArray<ATruGameObject*>>& ParentToChildrenMap);

};
