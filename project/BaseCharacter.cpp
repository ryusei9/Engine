#include "BaseCharacter.h"
#include <Object3DCommon.h>
void BaseCharacter::Initialize()
{
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();
}

void BaseCharacter::Update()
{
}

void BaseCharacter::Draw()
{
}
