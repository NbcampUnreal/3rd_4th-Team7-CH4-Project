// Fill out your copyright notice in the Description page of Project Settings.

#include "THItemRowSource.h"

UTHItemRowSource* UTHItemRowSource::Make(UObject* Outer, FName InRowName)
{
    if (!Outer)
    {
        Outer = GetTransientPackage(); // �ʿ� �� �κ��丮 ������Ʈ�� Outer�� �Ѱܵ� OK
    }

    // ������ϱ� ���� ���̽� �̸��� ������, �׻� ����ũ �̸��� ���
    const FName BaseName(*FString::Printf(TEXT("THItemRowSource_%s"), *InRowName.ToString()));
    const FName UniqueName = MakeUniqueObjectName(Outer, StaticClass(), BaseName);

    // RF_Transient: ����/PIE �� ���� �ּ�ȭ
    UTHItemRowSource* Obj = NewObject<UTHItemRowSource>(Outer, UniqueName, RF_Transient);
    Obj->RowName = InRowName;
    return Obj;
}