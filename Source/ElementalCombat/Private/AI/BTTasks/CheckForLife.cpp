// Property of Avrlon, all rights reserved.

#include "ElementalCombat.h"
#include "CheckForLife.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UCheckForLife::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	// Check the own AI life, if dead stop casting
	auto AIController = OwnerComp.GetAIOwner();
	auto Owner = AIController->GetPawn();



	// Check the enemy Life. If enemy is dead stop casting

	return EBTNodeResult::Succeeded;
}
