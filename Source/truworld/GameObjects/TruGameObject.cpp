// TruGameObject.cpp

#include "TruGameObject.h"
#include "truworld/Editor/EditorPlayerController.h"

ATruGameObject::ATruGameObject()
{
	PrimaryActorTick.bCanEverTick = false;

	// Initialize components
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	BoxMesh->SetupAttachment(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (BoxMeshAsset.Succeeded())
	{
		BoxMesh->SetStaticMesh(BoxMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Game/StarterContent/Materials/M_Basic_Wall.M_Basic_Wall"));
	if (Material.Succeeded())
	{
		BoxMesh->SetMaterial(0, Material.Object);
	}

	BoxMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
}

ATruGameObject::~ATruGameObject()
{
}

void ATruGameObject::OnSelected()
{
}

void ATruGameObject::OnDeselected()
{
}

void ATruGameObject::BeginPlay()
{
	Super::BeginPlay();
	NotifyEditorPlayerController();
}

void ATruGameObject::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	NotifyEditorPlayerController();
}

void ATruGameObject::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	NotifyEditorPlayerController();
}

void ATruGameObject::NotifyEditorPlayerController()
{
	if(!GetWorld())
		return;
	AEditorPlayerController* EditorController = Cast<AEditorPlayerController>(GetWorld()->GetFirstPlayerController());
	if (EditorController)
	{
		EditorController->OnGameObjectsRefreshed();
	}
}
