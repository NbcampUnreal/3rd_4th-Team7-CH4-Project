// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "UObject/Object.h"
#include "THItemRowSource.generated.h"

/*
    RowName�� ��Ʈ��ũ�� ��� �ٴϱ� ���� ��������.
    GAS���� �����Ƽ�� ����Ʈ�� UObject�� ��� �ٴϴµ�, ���ø����̼��̳� RPC���� ������ �ȵǴ� ���� ����. 
    RowName�� ���� UObject ���� Rowname �ؼ��ϰ� FTHItemData�� �ε� �� �� �ֵ��� ��
    ������Row�� Gas�����������̶� ��Ʈ��ũ�� ������ ���� ������ ���� Ŭ���� 
*/

// ������ RowName�� ��� ���� �����̳�
UCLASS()
class TREASUREHUNTER_API UTHItemRowSource : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly)
    FName RowName;

    // �浹 ���� �׻� ����ũ�ϰ� �̸� ���� -> ���� �̸����� �����ϸ� Rename on top of existing object crash 
    static UTHItemRowSource* Make(UObject* Outer, FName InRowName);
    
    virtual bool IsSupportedForNetworking() const override { return true; } // �̰� ���� NetGUID �����ؼ� Ŭ�� ���� �� ���� ���� �� �� �ֵ��� ���� 
};
