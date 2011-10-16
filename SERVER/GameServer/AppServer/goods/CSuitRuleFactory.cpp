#include "stdafx.h"
#include "CSuitRuleFactory.h"
#include "CGoods.h"
#include "CSuitRule.h"
#include "..\player.h"


map<int,CSuitRule*> CSuitRuleFactory::m_mRules;

CSuitRuleFactory::CSuitRuleFactory()
{
	// TODO: Add your specialized code here.
}

CSuitRuleFactory::~CSuitRuleFactory()
{
	// TODO: Add your specialized code here.
}

VOID CSuitRuleFactory::Release()
{
	map<int,CSuitRule*>::iterator ruleIter=m_mRules.begin();
	for(;ruleIter!=m_mRules.end();ruleIter++)
	{
		MP_DELETE(ruleIter->second);
	}
	m_mRules.clear();
}

BOOL CSuitRuleFactory::Serialize(vector<BYTE>* pStream, BOOL b)
{
	_AddToByteArray( pStream, static_cast<DWORD>( m_mRules.size() ) );
	map<int,CSuitRule*>::iterator it = m_mRules.begin();
	for( ; it != m_mRules.end(); it ++ )
	{
		_AddToByteArray( pStream, (long)(it -> first) );
		it -> second -> Serialize( pStream );
	}
	return TRUE;
}

BOOL CSuitRuleFactory::Unserialize(BYTE* pStream, LONG& lOffset, BOOL b)
{
	DWORD dwNum = _GetDwordFromByteArray( pStream, lOffset );
	for( DWORD i = 0; i < dwNum; i ++ )
	{
		long lVal=_GetLongFromByteArray(pStream,lOffset);
		CSuitRule* pRule=MP_NEW CSuitRule;
		pRule->Unserialize(pStream,lOffset);
		m_mRules[lVal]=pRule;
	}
	return TRUE;
}

//套装效果激活
BOOL CSuitRuleFactory::SuitActive(CPlayer* pPlayer,int nNum,CGoods* pGoods)
{
	BOOL bRet=FALSE;
	if(!pPlayer)
		return bRet;
	LONG lSuitId=pGoods->GetAddonPropertyValues(GAP_SUITID,1);
	LONG lSuitNumer=pGoods->GetAddonPropertyValues(GAP_SUIT_NUMBER,1);
	if(lSuitId==0)
		return bRet;
	

	map<int,CSuitRule*>::iterator ruleIter=m_mRules.find(lSuitId);
	if(ruleIter!=m_mRules.end())
	{	
		ruleIter->second->RuleInvalidate(pPlayer,nNum);
		nNum++;
		bRet=ruleIter->second->RuleActive(pPlayer,nNum);
		if(bRet)
		{/*
			CMessage msg(MSG_S2C_SUIT_ACTIVE);
			msg.Add(lSuitId);
			msg.Add((BYTE)nNum);
			msg.SendToPlayer(pPlayer->GetID());*/
		}
		
		//attr update
	}
	return bRet;	
}


//套装效果取消
BOOL CSuitRuleFactory::SuitInvalidate(CPlayer* pPlayer,int nNum,CGoods* pGoods)
{
	BOOL bRet=FALSE;
	if(!pPlayer)
		return bRet;
	LONG lSuitId=pGoods->GetAddonPropertyValues(GAP_SUITID,1);
	LONG lSuitNumer=pGoods->GetAddonPropertyValues(GAP_SUIT_NUMBER,1);
	if(lSuitId==0)
		return bRet;	
	map<int,CSuitRule*>::iterator ruleIter=m_mRules.find(lSuitId);
	if(ruleIter!=m_mRules.end())
	{
		bRet=ruleIter->second->RuleInvalidate(pPlayer,nNum);
		nNum--;
		BOOL bActived=ruleIter->second->RuleActive(pPlayer,nNum);
		if(bActived)
		{/*
			CMessage msg(MSG_S2C_SUIT_ACTIVE);
			msg.Add(lSuitId);
			msg.Add((BYTE)nNum);
		    msg.SendToPlayer(pPlayer->GetID());*/
		}
		
		//attr update
	}
	return bRet;
}


BOOL CSuitRuleFactory::Load(const CHAR* strPath)
{
	m_mRules.clear();

	CRFile* prfile = rfOpen(strPath);
	if(prfile == NULL)
	{		
		return FALSE;
	}

	stringstream stream;
	prfile->ReadToStream(stream);
	rfClose(prfile);	
	string strVal="";	
	while(ReadTo(stream, "SUIT_ID"))
	{
		int suit_id;
		stream >> suit_id;
		ReadTo(stream,"<SUIT_GOODS>");
		
		CSuitRule* pRule=MP_NEW CSuitRule;
		m_mRules[suit_id]=pRule;
		while(TRUE)
		{
			stream>>strVal;
			if(strVal=="</SUIT_GOODS>")
			{
				break;
			}
			int goods_id=atoi(strVal.c_str());
			stream>>strVal;//原始名
			pRule->m_mEquip[goods_id]=strVal;

		}
        bool bLoop=TRUE;
		while(bLoop)
		{
			ReadTo(stream,"NUM");
			int num;
			stream>>num;//件数
			pRule->m_mAttr[num]=new vector<CSuitRule::tagRuleValue*>;
			ReadTo(stream,"<ATTR>");
			while(TRUE)
			{
				stream>>strVal;
				if(strVal=="</ATTR>")
				{
					break;
				}
				else if(strVal=="</ATTR_END>")
				{
					bLoop=FALSE;
					break;
				}
				CSuitRule::tagRuleValue* pRuleValue=MP_NEW CSuitRule::tagRuleValue;
				pRuleValue->strType=strVal;
				stream>>pRuleValue->lVal;
				pRule->m_mAttr[num]->push_back(pRuleValue);

			}
		}		
	}	
	return TRUE;
}