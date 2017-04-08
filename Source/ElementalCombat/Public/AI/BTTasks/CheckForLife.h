// Property of Avrlon, all rights reserved.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CheckForLife.generated.h"

/**
 * 
 */
UCLASS()
class ELEMENTALCOMBAT_API UCheckForLife : public UBTTaskNode
{
	GENERATED_BODY()
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

	UPROPERTY(EditAnywhere, Category = "AIBlackboard")
	struct FBlackboardKeySelector IsGameOverKey;
	
};
