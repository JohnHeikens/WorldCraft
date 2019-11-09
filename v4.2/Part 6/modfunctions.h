#pragma once
#include "include.h"
struct fsetblock : public function
{
	fsetblock()
	{
		name = L"setblock";
		arguments = std::vector<variable*>();
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tbool));
		result = DBG_NEW variable(tvoid);
	}
	virtual void execute() const override;
};
struct fgetblock : public function
{
	fgetblock()
	{
		name = L"getblock";
		arguments = std::vector<variable*>();
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		result = DBG_NEW variable(tint);
	}
	virtual void execute() const override;
};
struct fsetblockrange : public function
{
	fsetblockrange()
	{
		name = L"setblockrange";
		arguments = std::vector<variable*>();
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tbool));
		result = DBG_NEW variable(tvoid);
	}
	virtual void execute() const override;
	
};
struct fsetsphere : public function
{
	fsetsphere()
	{
		name = L"setsphere";
		arguments = std::vector<variable*>();
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tdouble));
		arguments.push_back(DBG_NEW variable(tint));
		arguments.push_back(DBG_NEW variable(tbool));
		result = DBG_NEW variable(tvoid);
	}
	virtual void execute() const override;
	
};
struct frand : public function
{
	frand()
	{
		name = L"rand";
		arguments = std::vector<variable*>();
		result = DBG_NEW variable(tint);
	}
	virtual void execute() const override;

};
struct frandfp : public function
{
	frandfp()
	{
		name = L"randfp";
		arguments = std::vector<variable*>();
		result = DBG_NEW variable(tdouble);
	}
	virtual void execute() const override;

};