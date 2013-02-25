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
	quint64 itemStyle = 0;

	bool bNeedsComment = false;

	if(strInstructions[0].compare("call") == 0)
	{
		itemStyle |= COLOR_CALLS;
		bNeedsComment = true;
	}
	else if(strInstructions[0].compare("jmp") == 0	||
		strInstructions[0].compare("jnz") == 0		||
		strInstructions[0].compare("je") == 0		||
		strInstructions[0].compare("jl") == 0		||
		strInstructions[0].compare("jng") == 0		||
		strInstructions[0].compare("jb") == 0		||
		strInstructions[0].compare("ja") == 0		||
		strInstructions[0].compare("js") == 0		||
		strInstructions[0].compare("jna") == 0		||
		strInstructions[0].compare("jo") == 0		||
		strInstructions[0].compare("jc") == 0		||
		strInstructions[0].compare("jnc") == 0		||
		strInstructions[0].compare("jnb") == 0		||
		strInstructions[0].compare("jpe") == 0		||
		strInstructions[0].compare("jnp") == 0		||
		strInstructions[0].compare("jle") == 0		||
		strInstructions[0].compare("jz") == 0		||
		strInstructions[0].compare("jno") == 0		||
		strInstructions[0].compare("jne") == 0)
	{
		itemStyle |= COLOR_JUMP;
		bNeedsComment = true;
	}
	else if(strInstructions[0].compare("push") == 0 ||
		strInstructions[0].compare("pushf") == 0 ||
		strInstructions[0].compare("popf") == 0 ||
		strInstructions[0].compare("pushfq") == 0 ||
		strInstructions[0].compare("popfq") == 0 ||
		strInstructions[0].compare("pop") == 0)
	{
		itemStyle |= COLOR_STACK;
	}
	else if(strInstructions[0].compare("mov") == 0	||
		strInstructions[0].compare("movsb") == 0	||
		strInstructions[0].compare("stosw") == 0	||
		strInstructions[0].compare("stowb") == 0	||
		strInstructions[0].compare("movs") == 0		||
		strInstructions[0].compare("movsd") == 0	||
		strInstructions[0].compare("movsq") == 0	||
		strInstructions[0].compare("movsw") == 0)
	{
		itemStyle |= COLOR_MOVE;
	}
	else if(strInstructions[0].compare("add") == 0	||
		strInstructions[0].compare("xor") == 0	||
		strInstructions[0].compare("rol") == 0	||
		strInstructions[0].compare("ror") == 0	||
		strInstructions[0].compare("shl") == 0	||
		strInstructions[0].compare("shr") == 0	||
		strInstructions[0].compare("rcl") == 0	||
		strInstructions[0].compare("rcr") == 0	||
		strInstructions[0].compare("sar") == 0	||
		strInstructions[0].compare("sal") == 0	||
		strInstructions[0].compare("mul") == 0	||
		strInstructions[0].compare("sub") == 0)
	{
		itemStyle |= COLOR_MATH;
	}

	if(bNeedsComment)
		pDataRow->Comment = CreateSymbols(strInstructions[1].replace("h","").toULongLong(0,16));
	else
		pDataRow->Comment = "";

	pDataRow->itemStyle = itemStyle;
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