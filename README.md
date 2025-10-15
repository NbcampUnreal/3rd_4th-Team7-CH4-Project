# Treasure Hunter - Multiplayer 1 vs 1 Game

> UE5.5· GAS 기반 · AWS 전용 서버 · 온라인 세션(로비) 매칭 · 커스텀 이동(클라임/맨틀) · 캐릭터 상호작용 (머리밟기/밀치기) · 아이템 시스템(Ability/Effect/Tag) · 단계별 UI 흐름

게임 인트로 영상 보기: 
[Trasure Hunter Game Intro](https://youtu.be/36j_MGRbPsY)
<img width="1536" height="1024" alt="4" src="https://github.com/user-attachments/assets/0a430461-5f1a-445f-a308-9698b8e77f2a" />

어느 날 마을의 선술집에서 **“동틀 녘 첫 햇살에만 드러나는 별빛 수정** 소문이 들려온다.
소문을 들은 두 라이벌 버니와 마우스는 먼저 수정을 얻기 위해 **산을 오르는 경쟁**을 시작한다. 
상대를 밀치고, 머리를 밟고, 심지어는 먹물로 시야를 가리고, 상대를 기절시키면서 치열한 접전을 펼치며 정상으로 치닫는다.

과연 최후의 승자는 누구인가? 그리고 산의 정상에는 소문의 별빛 수정이 정말 존재할까?  

---

## 목차
[1) 개요 & 목표](#1-개요--목표)  
[2) 전반구조](#2-전반구조)  
[3) 네트워크 및 동기화 포인트](#3-네트워크-및-동기화-포인트)  
[4) 게임 진행 흐름](#4-게임-진행-흐름)  
[5) UI 흐름](#5-ui-흐름)  
[6) 아이템/인벤토리 시스템](#6-아이템인벤토리-시스템)  
[7) 캐릭터 조작/능력](#7-캐릭터-조작능력)  
[8) 트러블슈팅](#8-트러블슈팅)  
[9) 플레이 이미지/영상](#9-플레이-이미지영상)

 ---

 ### 1) 개요 & 목표 

 - 제목: Treasure Hunter
 - 장르/시점: 3인칭 액션 어드벤처 퍼즐 게임
 - 플레이 인원: 1 vs 1
 - GAS 구조
    - Ability: 발동/타겟팅/설치 등 로직
    - Effect: 수치 변경 (이속/점프/스태미나 조정 등)
    - Tag: 상태/지속시간/쿨다운/Active 플래그 관리
- 네트워크 동기화(버프/디버프 기믹, 애니메이션, 공격, 컴포넌트, 매치 스테이트)와 UI → 매칭/로딩 → HUD → 종료 UI의 전체 플레이 사이클 구현

---

### 2) 전반구조
> 핵심 클래스

- GameInstance – 세션 생성/탐색/참가, 초대 수락, 클라이언트 트래블

- GameMode / GameState – 매치 스테이트(Wait/Match/Loading/Play/Finish), 리매치 태그 동기화

- PlayerController (Play) – HUD·인벤토리 바인딩, 페이즈 반응, 리매치 응답

- PlayerController (Title) – 메인/매칭/로딩 위젯 전환

- PlayerState – ASC/AttributeSet 초기화, 스타트업 어빌리티 지급, 닉네임 복제

- Character – 입력·상호작용(E), Sprint/Push/Mantle/Climb

- Movement component – 커스텀 이동(MOVE_Climb), 표면 트레이스/정렬/모션워핑

- Parkour Component – 맨틀 검출/워프 타깃 계산(th.Debug.Mantle CVar)

- FTHItemData(DataTable Row) - 드랍 풀/가중치/지속/UI/Ability/스폰클래스

- ATHItemDataManager - 아이콘 캐시, Row 조회, 현재 1등 판단 헬퍼

- ATHItemBox - 상자 오픈→드랍 결정→아이템 스폰(Multicast 연출)

- ATHBaseItem - 필드 드랍 아이템(줍기)

- UTHItemInventory - 슬롯(1/2) 보관 및 서버에서 Ability 부여·즉발, OwnerOnly 

  ---

### 3) 네트워크 및 동기화 포인트

**네트워크 핵심기능**
> OSS-Steam 기반 (Steam UniqueId 기반) Listen 서버를 생성하여 플레이어가 세션을 Host하고 스팀 계정으로 초대/참가 하여 게임 진행 
- Steam UniqueNetId 확보하여 스팀 닉네임 기반 플레이 

**OnlineSubsystem 사용**
- Host: UTHGameInstance::HostSession()에서 Steam 세션 생성 → 서버는 ServerTravel로 Play 맵 로드
- Join: JoinServer()로 Steam 세션/로비 검색 → 임의(또는 기준) 세션 참가
- 클라 이동: bUseSeamlessTravel=true + GetSeamlessTravelActorList()에서 PlayerState를 넘겨, 닉네임/ASC 등 상태 유지

**동기화 포인트**
- 공격 동기화: UTHPushAbility 에서 NetExecutionPolicy=ServerOnly (서버 판정)
- 아이템 상자: ATHItemBox 상자 오픈은 서버 처리 → 애니메이션/파괴는 Multicast로 동기화
- 아이템 사용: 아이템 사용 시 서버가 Ability 부여 → TryActivate() 성공 시 Client Notify로 HUD에 반영
- 인벤토리: UTHItemInventory 슬롯 상태 OwnerOnly 복제, 사용/아이콘 이벤트는 Client 처리
- 애니메이션: MOVE_Climb용 법선 **CurrentClimbableSurfaceNormal** Replicated → 몽타주 재생은 서버 계산 기반으로 클라 처리

---

### 4) 게임 진행 흐름

``` SCSS
Wait (대기실)
→ Match (슬롯 선택 / 레디)
→ Loading (레벨 로딩)
→ Play
→ Finish (승패 / 리매치)
```
- Match: 플레이어가 슬롯0 (토끼) 슬롯1 (쥐) 중 하나를 골라 준비 → bSlotsLockedIn=true → LoadGame()
- Play: 0.1초 간격으로 진행도 및 순위 계산 (평지 / 클라임 구간 가중 합산) → HUD 클라임 바와 순위 업데이트
- Finish: 리매치가 성사되면 레벨 리로드 || 거점/이탈/타임아웃 시 메인으로 복귀 가능

---

### 5) UI 흐름
- THMainMenuWidget: 게임 시작 (자동매칭)
- THMatchmakingWidget: 캐릭터 선택 (버니 || 마우스)
- THLoadingWidget: 레벨 로딩 진행도(UI는 fake 로딩처리 → 스트리밍·텍스처 안정화 대기 후 서버에 “준비완료” 알림)
- THPlayerHUDWidget: 
  - 스태미나/이속/점프력 바 [좌측 하단]
  - 인벤토리 슬롯(1/2) 아이콘 [우측 하단]
  - 풀 스크린 오버레이 [중앙]
  - 클라인 진행도 (나/상대) [우측 중앙]
  - 순위 업데이트 [좌측 상단]
  - 아이템 사용은 이펙트/우측상단 아이콘/좌측 하단 바에서 시각 처리
- THGameOverWidget: 승패/리매치 모달(대기·거절·이탈·타임아웃 메시지)

> HUD는 PlayerState의 **ASC/AttributeSet에 바인딩**하여 실시간 반영 (**예**: Sprinting 태그가 부착되어 있을 때에 스태미나 바가 줄어들게 처리)
```c++
void UTHPlayerHUDWidget::BindToAbilitySystem(UAbilitySystemComponent* InASC, const UTHAttributeSet* InAttr)
{
...

	RefreshStaminaGeometry();
	Recompute(EBuffKind::Speed);
	Recompute(EBuffKind::Jump);

	BindAttributeDelegates();

	TagSprint = TAG_State_Movement_Sprinting;
	bIsSprinting = AbilitySystem->HasMatchingGameplayTag(TagSprint);
	{
		auto& Ev = AbilitySystem->RegisterGameplayTagEvent(TagSprint, EGameplayTagEventType::NewOrRemoved);
		SprintingTagHandle = Ev.AddUObject(this, &ThisClass::OnSprintingTagChanged);
	}
}
void UTHPlayerHUDWidget::BindAttributeDelegates()
{
	if (!AbilitySystem || !Attr) return;

	MaxStaminaChangedHandle =
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetMaxStaminaAttribute())
		.AddUObject(this, &ThisClass::OnMaxStaminaChanged);

	CurrentStaminaChangedHandle =
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attr->GetStaminaAttribute())
		.AddUObject(this, &ThisClass::OnCurrentStaminaChanged);
...

}
```

---

### 6) 아이템/인벤토리 시스템
> 데이터 스키마 - FTHItemData

| 필드                         | 설명                                                                             |
| -------------------------- | ------------------------------------------------------------------------------ |
| `ItemName`, `ItemIcon`     | 표시용 텍스트 & 아이콘(Soft)                                                   |
| `ItemDropType`             | `Winner` / `Loser` / `Common` (드랍 풀 구분)                                   |
| `DropWeight`               | 풀 내 가중치                                                                   |
| `UiIndicator`              | `InventoryProgressBarBuff` / `TopRightInventoryBuffIcon` / `FullScreenOverlay` |
| `UseKind`, `DurationSec`   | 즉시/지속/설치/타겟형 + 지속시간                                               |
| `VictimOverlayWidgetClass` | 디버프용 전체화면 위젯(Ink 등)                                                 |
| `GameplayAbilityClass`     | 사용 시 부여·발동할 Ability                                                    |
| `BaseItemClass`            | 상자에서 떨어지는 액터 클래스                                                 |

**드랍 로직**
- ATHGameModeBase::AccumulatePlayerDistance()에서 토끼가 리드 중인지 확인
- ATHItemDataManager::WhoWInner(PC) → ATHItemBox::RandomItemGenerate(PC)가 1등인지 2등인지에 따라 DropWeight 가중치 랜덤으로 Row 선택
- 이길수록 수비적 버프 아이템 vs 질수록 상대 디버프 아이템 확률 높아짐

---

### 7) 캐릭터 조작/능력
- 이동: WASD / 시점: 마우스
- 점프: Space Bar
- 스프린트: Shift (Ability.Sprint)
- 푸시: V (Ability.Push)
- 맨틀: 마우스 우클릭 (Ability.Mantle)
- 클라임: 마우스 좌클릭
- 상호작용: E
- 아이템 사용: 1/2

**맨틀&클라이밍**
- 맨틀: 전방 벽/난간을 파쿠르 컴포넌트가 트레이스 판정 → 캡슐로 착지 공간 확인
  - 성공: Motion Warping 타깃 (Up/Forward) 계산해 UTHMantleAbility 전달 → 충돌/이동상태 변환 → 맨틀 몽타주 재생
  - 완료/중단: 워핑 타깃 제거
- 클라이밍: 커스텀 이동모드 MOVE_Climb으로 전환하여 전방 박스 트레이스 판정 → 눈높이로 오를 수 있는 면 확인
  - 성공: 이동 중 표면 법선으로 스냅/정렬하고, 난간에 도달하면 맨틀 실행

**밀치기&밟기**
- 밀치기: ServerInitiated로 캐릭터 전방에 적 인식(방향 벡터의 수평 정규화) → LaunchCharacter(Dir * PushForce + ZForce) + 몽타주 재생
- 밟기: ServerOnly로 **TAG_Event_Hit_Falling** 시에 서버 판정으로 충돌 판단 → 시전자는 LaunchCharacter(StompJumpForce) + 상대는 Stun GameplayEffect 적용 및 GameplayCue로 기절이펙트 실행 

---

### 8) 트러블슈팅 (dev 기준) 

| Name | Commit(s)            | Type          | Problem                                 | Cause                                           | Fix                                                                                       | Verification                                |
| ---- | -------------------- | ------------- | --------------------------------------- | ----------------------------------------------- | ----------------------------------------------------------------------------------------- | ------------------------------------------- |
| 김동권     | `18161fb`, `2157564` | fix           | Mantle 중 Push로 캔슬, Mantle 종료 후 스태미나 미회복 | 어빌리티 상호 배제/선행 조건 미정의, Regen 트리거 누락              | `BlockedAbilitiesWithTag`로 Mantle 중 타 어빌리티 차단, Mantle `EndAbility`에서 **Stamina Regen** 적용 | Mantle 중 Push 입력 무시, Mantle 종료 후 스태미나 자연 회복 |
| 임형균    | `3b4a5df`, `650b3f8` | feature       | Gameplay Cue / Niagara가 패키징 후 미재생       | Soft 참조로 쿠킹 제외                                  | 관련 에셋 **강제 쿠킹** 목록에 추가                                                                    | 패키지 빌드 후 이펙트/큐 정상 재생                        |
| 심효종     | `5abcfde`, `5fab77a` | fix / content | 인벤토리 아이콘 간헐적 미표시, 아이템 스폰 시 프레임 드랍       | UI 아이콘 `TWeakObjectPtr`로 GC 타이밍 이슈, 과도한 아이템 콜리전 | 아이콘 참조를 `TObjectPtr`로 전환, **복잡한 콜리전 제거**                                                  | 아이콘 안정 표시, 연속 스폰 시 프레임 드랍 감소                |
| 명주이     | `870c236`            | fix           | 벽 관통 상태로 클라이밍                           | 캡슐 반경 과소, Snap/회전 보정 오차                         | 캡슐 **Radius 20→47**, Snap 로직/`GetClimbRotation` 보정                                        | 벽 관통 재현 불가, 표면 정렬 안정화                       |
| 천재우     | `cbd28df`            | feature       | 클라이밍 손/발 접지 불자연                         | IK 보정 부재                                        | **Foot IK** 도입으로 접지/접벽 보정                                                                 | 경사/요철 벽면에서도 자연스러운 접지 및 포즈 유지                |
| 김태훈    | N/A       | fix  | 맵 스트리밍/페이징 시 끊김과 과도한 생성 지연 | 레벨에 **PCG 볼륨**이 과도하게 배치되어 불필요한 생성/업데이트 발생 | 불필요한 PCG 볼륨 **정리/삭제**, 꼭 필요한 최소 볼륨만 유지 | 이동 시 히치 감소, 타일/셀 페이징 안정화 |

---

### 9) 플레이 이미지/영상
게임 소개 자료:
[Treasure Hunter Game Presentation](https://www.canva.com/design/DAG0E8weHBM/rHJ6QK_KS1D-e1dv8uBQbw/edit)

<img width="2880" height="1601" alt="Pub" src="https://github.com/user-attachments/assets/9dcb7cae-a961-424a-8855-edfc1f9fba72" />
<img width="2575" height="1576" alt="Mountain001" src="https://github.com/user-attachments/assets/4f14d93a-6a97-429d-8b53-5977a20a2d8e" />
<img width="1936" height="1048" alt="bunny_use_item_jump_01" src="https://github.com/user-attachments/assets/5fa4efa7-16f9-446f-a52f-98eca42ab6bc" />
<img width="1920" height="1080" alt="mouse_sprint_00" src="https://github.com/user-attachments/assets/09917b66-49fc-48c6-9eb2-5e788f1cd7af" />
<img width="1920" height="1080" alt="mouse_closedchest_00" src="https://github.com/user-attachments/assets/ca0fab8a-89b0-4dc3-92b3-2a1c528c1262" />
<img width="1920" height="1080" alt="mouse_clmib_02" src="https://github.com/user-attachments/assets/e0a28166-f007-42be-949c-40262a89f1fb" />
