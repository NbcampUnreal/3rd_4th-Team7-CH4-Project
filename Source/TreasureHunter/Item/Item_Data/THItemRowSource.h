// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "UObject/Object.h"
#include "THItemRowSource.generated.h"

/*
    RowName을 네트워크로 들고 다니기 위해 만들어놓음.
    GAS에선 어빌리티나 이펙트를 UObject로 들고 다니는데, 레플리케이션이나 RPC에서 참조가 안되는 문제 있음. 
    RowName을 담은 UObject 만들어서 Rowname 해석하고 FTHItemData를 로드 할 수 있도록 함
    아이템Row를 Gas파이프라인이랑 네트워크에 보내기 위해 만들어둔 래퍼 클래스 
*/

// 아이템 RowName만 담는 얇은 컨테이너
UCLASS()
class TREASUREHUNTER_API UTHItemRowSource : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly)
    FName RowName;

    // 충돌 없이 항상 유니크하게 이름 생성 -> 같은 이름으로 생성하면 Rename on top of existing object crash 
    static UTHItemRowSource* Make(UObject* Outer, FName InRowName);
    
    virtual bool IsSupportedForNetworking() const override { return true; } // 이걸 쓰면 NetGUID 생성해서 클라 서버 간 참조 복제 할 수 있도록 설정 
};
