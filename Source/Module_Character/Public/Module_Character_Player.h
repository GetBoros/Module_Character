#pragma once

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AbilitySystemInterface.h"

#include "GameFramework/Character.h"
#include "Module_Character_Player.generated.h"

//-------------------------------------------------------------------------------------------------------------
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
//-------------------------------------------------------------------------------------------------------------
UCLASS() class UMyCharacterAttributes : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMyCharacterAttributes();

	UPROPERTY(BlueprintReadOnly, Category = "Attributes") FGameplayAttributeData Health;
	UPROPERTY(BlueprintReadOnly, Category = "Attributes") FGameplayAttributeData Mana;
	UPROPERTY(BlueprintReadOnly, Category = "Attributes") FGameplayAttributeData Damage;
	UPROPERTY(BlueprintReadOnly, Category = "Attributes") FGameplayAttributeData Experience;

	// Репликация
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Макросы для доступа к `Experience`
	ATTRIBUTE_ACCESSORS(UMyCharacterAttributes, Experience)
};
//-------------------------------------------------------------------------------------------------------------
UCLASS() class ULockpickAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	ULockpickAbility();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	void GiveExperience(AActor* PlayerActor);
};
//-------------------------------------------------------------------------------------------------------------
class USpringArmComponent;
class UCameraComponent;
class UAbilitySystemComponent;
//-------------------------------------------------------------------------------------------------------------
UCLASS() class MODULE_CHARACTER_API AAModule_Character_Player : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAModule_Character_Player();

	virtual void BeginPlay();
	virtual void NotifyControllerChanged();
	virtual UAbilitySystemComponent *GetAbilitySystemComponent() const;

	void Look(const FVector2D look_axis_vector);
	void Zoom(const float step_offset);
	void Camera_Exit();  // Restore Boom state || Menu || Q Button |
	void Interact();

	bool Is_State_Camera;  // Remove to switch if have more states

	UFUNCTION(BlueprintCallable) void Camera_Switch(const FVector location, const FRotator rotation);

	UPROPERTY() UMyCharacterAttributes *AttributeSet;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities") UAbilitySystemComponent *Ability_System_Component;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true") ) USpringArmComponent *Camera_Boom;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true") ) UCameraComponent *Camera_Follow;

};
//-------------------------------------------------------------------------------------------------------------





// !!! "GameplayAbilities", "GameplayTags", "GameplayTasks" to .cs

//------------------------------------------------------------------------------------------------------------
#pragma region HELP
/*
	- Player #include "AbilitySystemInterface.h" IAbilitySystemInterface to implement UAbilitySystemComponent
	- UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities") UAbilitySystemComponent *Ab_Sy_Co;
*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------
#pragma region Gameplay_Effects
/*
	- Duration:
		- Gameplay Effects can apply instantly (such as Health decreasing when attacked), over a limited duration (such as a movement speed boost that lasts for a few seconds),
		or infinitely (such as a character naturally regenerating magic points over time). Effects that have a non-instant duration can apply themselves at different intervals.
		These intervals can change how the Effect works regarding gameplay and the timing of audio or visual effects.
	- Components:
		-Gameplay Effect Components define how a Gameplay Effect behaves. For a complete list of available components See [#GameplayEffectComponents]
	- Modifiers:
		- Modifiers determine how the Gameplay Effect interacts with Attributes. This includes mathematical interactions with Attributes themselves,
		such as increasing an armor rating attribute by 5 percent of its base value, and includes Gameplay Tag requirements to execute the Effect.
	- Executions:
		- Executions use the UGameplayEffectExecutionCalculation to define custom behaviors that the Gameplay Effect has when it executes.
		These are particularly useful for defining complex equations that aren't adequately covered by Modifiers.
	-Gameplay Cues:
		- Gameplay Cues are a network-efficient way to manage cosmetic effects, like particles or sounds, that you can control with the Gameplay Ability System.
		Gameplay Abilities and Gameplay Effects can trigger Gameplay cues.
		- Gameplay Cues act through four main functions that can be overridden in native or Blueprint code:
			- On Active
			- While Active
			- Removed
			- Executed (used only by Gameplay Effects).
		- All Gameplay Cues must be associated with a Gameplay Tag that starts with GameplayCue, such as GameplayCue.ElectricalSparks or GameplayCue.WaterSplash.Big.
	- Stacking
		- Stacking refers to the policy of applying a buff or debuff (or Gameplay Effect) to a target that already carries it.
		It also covers handling overflow, where a new Gameplay Effect is applied to a target that is already fully saturated with the original
		Gameplay Effect (such as a poison meter that builds up, resulting in damage-over-time poison only after it overflows).
		- The system supports a wide variety of Stacking behaviors, such as:
			- Building effects until a threshold is broken.
			- Maintaining a "stack count" that increases with each fresh application up to a maximum limit.
			- Resetting or appending time on a limited-time Effect.
			- Applying multiple instances of the Effect independently with individual timers.

	- Gameplay Effect Components:
		- UChanceToApplyGameplayEffectComponent:
			- Probability that the Gameplay Effect is applied.
		- UBlockAbilityTagsGameplayEffectComponent	
			- Handles blocking the activation of Gameplay Abilities based on Gameplay Tags for the Target Actor of the owner Gameplay Effect.
		- UAssetTagsGameplayEffectComponent	
			- Tags the Gameplay Effect Asset owns. These do not transfer to any Actors.
		- UAdditionalEffectsGameplayEffectComponent	
			- Add additional Gameplay Effects that attempt to activate under certain conditions (or no conditions.)
		- UTargetTagsGameplayEffectComponent	
			- Grant Tags to the Target (sometimes referred to as the Owner) of the Gameplay Effect.
		- UTargetTagRequirementsGameplayEffectComponent	
			- Specify tag requirements that the Target (owner of the Gameplay Effect) must have if this GE should apply or continue to execute.
		- URemoveOtherGameplayEffectComponent	
			- Remove other Gameplay Effects based on certain conditions.
		- UCustomCanApplyGameplayEffectComponent	
			- Handles the configuration of a CustomApplicationRequirement function to see if this GameplayEffect should apply.
		- UImmunityGameplayEffectComponent	
			- Immunity is blocking the application of other GameplayEffectSpecs.
*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------
#pragma region Gameplay_Abilities
/*
	- GiveAbility: 
		- Specifies the Ability to add with an FGameplayAbilitySpec, and returns an FGameplayAbilitySpecHandle. Only the server can give or revoke Abilities.

	- GiveAbilityAndActivateOnce: 
		- Specifies the Ability to add with an FGameplayAbilitySpec, and returns an FGameplayAbilitySpecHandle.
		Because only the server can give Abilties, the Ability must be instanced and able to run on the server.
		After attempting to run the Ability, a FGameplayAbilitySpecHandle will be returned.
		If the Ability did not meet the required criteria, or if it could not execute, the return value will be invalid and the Ability System Component will not be granted the Ability.
		- Similar to giving abilities, only the server can remove abilities.
		The following functions can revoke access to an Ability from an Ability System Component, using the FGameplayAbilitySpecHandle that was returned when the Ability was granted:

	- ClearAbility:
		- Removes the specified Ability from the Ability System Component.

	- SetRemoveAbilityOnEnd:
		- Removes the specified Ability from the Ability System Component when that ability is finished executing.
		If the Ability is not executing, it will be removed immediately.
		If the Ability is executing, its input will be cleared immediately so that the player cannot reactivate or interact with it any further.

	- ClearAllAbilities:
		-Removes all Abilities from the Ability System Component. This function is the only one that does not require an FGameplayAbilitySpecHandle.

	- Base Usage:
	- CanActivateAbility:
		- Lets the caller know whether or not an Ability is available for execution without attempting to execute it.
		For example, your user interface may need to gray out and deactivate icons that the player can't use, or play a sound or particle effect on the character to show that a certain Ability is available.

	- CallActivateAbility:
		- Executes the game code associated with the Ability, but does not check to see if the Ability should be available.
		This function is usually called in cases where some logic is needed between the CanActivateAbility check and the execution of the Ability.
		The main code that users need to override with their Ability's custom functionality is either the C++ function called ActivateAbility, or the Blueprint Event called Activate Ability.

	- CommitAbility():
		-if called from within Activate, will apply the cost of executing the Ability, such as by subtracting resources from Gameplay Attributes (such as "magic points", "stamina",
		or whatever fits your game's systems) and applying cooldowns.

	- CancelAbility:

	- TryActivateAbility: 
		- is the typical way to execute Abilities. This function calls CanActivateAbility to determine whether or not the Ability can run immediately, followed by CallActivateAbility if it can.

	- EndAbility:
		- (in C++) or the End Ability node (in Blueprints) shuts the Ability down when it is finished executing.
		If the Ability was canceled, the UGameplayAbility class will handle this automatically as part of the cancelation process,
		but in all other cases, the developer must call the C++ function or add the node into the Ability's Blueprint graph.
		Failing to end the ability properly will result in the Gameplay Ability System believing that the Ability is still running,
		and can have effects such as preventing future use of the Ability or any Ability that it blocks.
		For example, if your game has a "Drink Health Potion" Gameplay Ability that doesn't end properly,
		the character using that Ability will be unable to take any action that drinking a health potion would prevent, such as drinking another potion, sprinting, climbing ladders, and so on.
		This Ability blockage will continue indefinitely, since the Gameplay Ability System will think that the character is stil busy drinking the potion.

	- TAGS:
		- Gameplay Tags can help to determine how Gameplay Abilities interact with each other.
		Each Ability possesses a set of Tags that identify and categorize it in ways that can affect its behavior,
		as well as Gameplay Tag Containers and Gameplay Tag Queries to support these interactions with other Abilities.

		- Cancel Abilities With Tag
			-Cancels any already-executing Ability with Tags matching the list provided while this Ability is executing.
		- Block Abilities With Tag
			- Prevents execution of any other Ability with a matching Tag while this Ability is executing.
		- Activation Owned Tags
			- While this Ability is executing, the owner of the Ability will be granted this set of Tags.
		- Activation Required Tags
			- The Ability can only be activated if the activating Actor or Component has all of these Tags.
		- Activation Blocked Tags
			- The Ability can only be activated if the activating Actor or Component does not have any of these Tags.
		- Target Required Tags
			- The Ability can only be activated if the targeted Actor or Component has all of these Tags.
		- Target Blocked Tags
			- The Ability can only be activated if the targeted Actor or Component does not have any of these Tags.

	- Replication:
		- Local Predicted:
		- Local Only:
		- Server Initiated:
		- Sever Only:

	- Instancing Policy, The three supported instancing types supported are:
		- Instanced per Execution: (ULTIMATE(DOTA 2 ) )
		- Instanced per Actor:
			- Each Actor will spawn one instance of this Ability when the Ability is first executed, and future executions will reuse it.
			This creates the requirement to clean up member variables between executions of the Ability, but also makes it possible to save information across multiple executions.
			Per-Actor is ideal for replication, as the Ability has a replicated Object that can handle variable changes and RPCs,
			but does not waste network bandwidth and CPU time spawning a new Object every time it runs.
			In larger-scale situations, this policy performs well, since large numbers of Actors using the Ability (for example, in a big battle) will only spawn Objects on their first Ability use.
		- Non-Instanced: 
			- This is the most efficient instancing policy in all categories. The Ability will not spawn any Object when it runs, and will instead use the Class Default Object.
			However, this efficiency introduces several restrictions. First, this policy uniquely requires that the Ability is written entirely in C++, as Blueprint Graphs require an Object instance.
			You can create Blueprint classes of a non-instanced Ability, but only to change the default values of exposed Properties.
			Further, the Ability must not change member variables or bind Delegates during its execution of the Ability. The Ability also cannot replicate variables or handle RPCs.
			This should be used only for Abilities that require no internal variable storage (although setting Attributes on the user of the Ability is possible) and don't need to replicate any data.
			It is especially well-suited to Abilities that run frequently and are used by many characters, such as the basic attack used by units in a large-scale RTS or MOBA title.
*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------
#pragma region Gameplay Attributes and sets
/*
	- Gameplay Attributes
		- FGameplayAttributesData
	- Attribute Sets
		- UAttributeSet
	In some cases, Gameplay Attributes can exist without an Attribute Set.
	This generally indicates that a Gameplay Attribute has been stored on an Ability System Component that does not have an Attribute Set containing the appropriate type of Gameplay Attribute.
	This is not recommended, because the Gameplay Attribute will have no defined behaviors that interact with any part of the Gameplay Ability System other than being storage for a floating-point value.


*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------

// GAS
#pragma region Gameplay_Ability(GA)
/*
	- Self-contained logic for a gameplay mechanic. BP or C++
	- Not just literal character abilities.
		- Move to, open chest, sprint, reaction behavior.
*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------
#pragma region Gameplay_Effects(GE)
/*
	- Effects of your GA, modifying your attributes and tags.
	- Can be: Infinity, Instant or Duration.
	- Data object.
*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------
#pragma region Gameplay_Tags(GT)
/*
	- Like actors tags(arguably better) 
	- Reusable to prevent human error and allow for quick selection. Stored in project settings.
	- Hierarchical.
	- Great matching system.
	- Extremely powerful.
	- Exist outside GAS plugin.

*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------
#pragma region Gameplay_Cue(GC)
/*
	- Essential wrapper to FX.
	- Come in two different variants:
		- Burts and Actor.
	- Generally all the client only, audio visual elements we don`t want to spawn on a dedicated server.

*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------
#pragma region GAS_Character_Setup
/*
	- Implement IAbilitySystemInterface (#include "AbilitySystemInterface.h")
	- Has UAbilitySystemComponent *AbilitySystemComponent; (#include "AbilitySystemComponent.h")
		- AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent") );
	- Has own UMyAttributeSet *AttributeSet; (#include "AttributeSet.h")
		- AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet") );

 - SPAWN:
	- AbilitySystemComponent->InitAbilityActorInfo(AActor *, AActor *);
	- Initialize Attributes.
	- Listen for tags of interests (via UASyncTaskGameplayTagAddedRemoved);
	- Add startup effects.
	- Bind to ability activation failed callback // for debug

- Begin Play:
	- Listen for health attribue changes.
	- Bind to ASC->OnImmunityBlockGameplayEffectDelegate.

- Attribute Sets:
	- Primary attributes - Core RPG stats that most thing have.
		- Health, MaxHealth, Stamina, MaxStamine.
	- Secondary Attributes - In depth RPG stat not needed by all entities.
		- Elemental resistance, bonuses, current elemental build up`s.
	- Tertiary Attributes - Bonuses ( mainly uses by Character).
		- Interaction speed bonuses, other specific bonuses and stats.
	- Weapon architype specific attributes.
		- Charge percent, spread, charge count.

*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
#pragma region UAttributeSet V
/*
* #include "AttributeSet.h"

	- FGameplayAttributeData ( Used in UAttributeSet like smart float ) :
		- Health, Mana, Stats Any.
		- Can(Must) be wrapped by FGameplayAttribute.
	
	- FGameplayAttribute ( supp struct to handle FGameplayAttributeData ) :
		- need write func to get reference at FGameplayAttributeData
		- or use macros ( ATTRIBUTE_ACCESSORS )
		- can get numeric, name, owner, property ref.
		- attributes have != or == comparsions
		
	- UAttributeSet
		- should create sub from those, stored FGameplayAttributeData, define the set of all GameplayAttributes.
		- added to the actors as subobjects, and then registered with the AbilitySystemComponent.
		- safe access to controll FGameplayAttribute || FGameplayAttributeData.
		- can get array of all property GetAttributesFromSetClass
		- have info about owners( ASC, Actor(info)
		- ShouldInitProperty() can disable attribute by some time, hidden attribute, or enemy don`t have this attribute
		- GE_(GameplayEffect execute to UAttributeSet) Pre | Post Execute :
			- can check if have imune to effect or something else, resist, change modifire, magnitude
			- Data.SetMagnitude(0.0f);  // Have imune to effect
			- PostGameplayEffectExectute if health == 0 now character can be disabled
		- PreAttributeChange() or Post :
			- Need to clamp max attribute value like health_max 100 if 100, 99 + heal(50) must be 100.
		- PreAttributeBaseChange() | Post :
			- Called when baseAttribute value changed, can again use clamp or what we need.
			- Armor with stats, Weapon, or esle.
		- Agregators calls when execute GE at attributeSet :
			- instant, duration, infitnity else (health + 10).

	- MACROSES :
		- Better do not use
*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------
#pragma region UAbilitySystemComponent X
/*
	- A component to easily interface with the 3 aspects of the AbilitySystem :
		- GameplayAbilities | GameplayEffects | GameplayAttributes.
		- Delegates personal count 6 six.
		- EGameplayEffectReplicationMode | how to replicate to client
		- 
	
	- GiveAbility
*/
#pragma endregion
//------------------------------------------------------------------------------------------------------------
