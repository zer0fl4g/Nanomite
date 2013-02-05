#include "clsSymbolAndSyntax.h"
#include "clsHelperClass.h"

#include <string>

using namespace std;

clsSymbolAndSyntax::clsSymbolAndSyntax(HANDLE hProc)
{
	_hProc = hProc;
}

clsSymbolAndSyntax::~clsSymbolAndSyntax()
{

}

bool clsSymbolAndSyntax::CreateDataForRow(DisAsDataRow *pDataRow)
{
	QStringList strInstructions = pDataRow->ASM.split(" ");

	/* "OFFSET COLOR:reserved:Syntax Highlight:Comment"  */
	quint64 itemStyle = (IsOffsetAnBP(pDataRow->Offset.toULongLong(0,16)) ? COLOR_BP : 0);

	bool bNeedsComment = false;

	if(strInstructions[0].compare("call") == 0)
	{
		itemStyle |= COLOR_CALLS;
		bNeedsComment = true;
	}
	else if(strInstructions[0].compare("jmp") == 0 ||
		strInstructions[0].compare("jnz") == 0 ||
		strInstructions[0].compare("je") == 0 ||
		strInstructions[0].compare("jl") == 0 ||
		strInstructions[0].compare("jng") == 0 ||
		strInstructions[0].compare("jne") == 0)
	{
		itemStyle |= COLOR_JUMP;
		bNeedsComment = true;
	}
	else if(strInstructions[0].compare("push") == 0 ||
		strInstructions[0].compare("pushf") == 0 ||
		strInstructions[0].compare("popf") == 0 ||
		strInstructions[0].compare("pop") == 0)
	{
		itemStyle |= COLOR_STACK;
	}
	else if(strInstructions[0].compare("mov") == 0 ||
		strInstructions[0].compare("movsb") == 0 ||
		strInstructions[0].compare("stosw") == 0 ||
		strInstructions[0].compare("stowb") == 0 ||
		strInstructions[0].compare("movsw") == 0)
	{
		itemStyle |= COLOR_MOVE;
	}

	if(bNeedsComment)
		pDataRow->Comment = CreateSymbols(pDataRow->Offset.toULongLong(0,16));

	pDataRow->itemStyle = quint64(itemStyle);
	return true;
}

QString clsSymbolAndSyntax::CreateSymbols(quint64 FuncOffset)
{
	wstring sFuncName, sModName;

	clsHelperClass::LoadSymbolForAddr(sFuncName,sModName,FuncOffset,_hProc);
	
	if(sFuncName.length() > 0 && sModName.length() > 0)
		return QString().append(QString::fromStdWString(sModName)).append(".").append(QString::fromStdWString(sFuncName));
	else if(sModName.length() > 0 && sFuncName.length() == 0)
		return QString().append(QString::fromStdWString(sModName)).append(".").append(QString("%1").arg(FuncOffset,16,16,QChar('0')));
	return "";
}

bool clsSymbolAndSyntax::IsOffsetAnBP(quint64 Offset)
{
	return false;
}