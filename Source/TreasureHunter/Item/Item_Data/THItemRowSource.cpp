// Fill out your copyright notice in the Description page of Project Settings.

#include "THItemRowSource.h"

UTHItemRowSource* UTHItemRowSource::Make(UObject* Outer, FName InRowName)
{
    if (!Outer)
    {
        Outer = GetTransientPackage(); // 필요 시 인벤토리 컴포넌트를 Outer로 넘겨도 OK
    }

    // 디버깅하기 좋은 베이스 이름을 만들어둠, 항상 유니크 이름을 사용
    const FName BaseName(*FString::Printf(TEXT("THItemRowSource_%s"), *InRowName.ToString()));
    const FName UniqueName = MakeUniqueObjectName(Outer, StaticClass(), BaseName);

    // RF_Transient: 세션/PIE 간 잔존 최소화
    UTHItemRowSource* Obj = NewObject<UTHItemRowSource>(Outer, UniqueName, RF_Transient);
    Obj->RowName = InRowName;
    return Obj;
}