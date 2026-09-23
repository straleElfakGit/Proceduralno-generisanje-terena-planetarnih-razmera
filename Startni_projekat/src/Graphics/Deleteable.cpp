#include "Deleteable.h"

Deleteable::Deleteable() : deleted(false) {}

Deleteable::~Deleteable() {}

void Deleteable::Delete()
{
	if (!deleted) {
		DeleteSpecific();
		deleted = true;
	}
}