#pragma once                //��//��
#ifndef __C2S_CONNECT_H__   //��//��
#define __C2S_CONNECT_H__   //��//��
#pragma pack(push, 1)       //��//��
//������������������������������//��
//�ļ����ƣ�C2S_Connect.h       //��
//����������ƽ̨C2S���������   //��
//ƽ̨��أ�Windows             //��
//����������������������������������
// C2S���Ӱ�
#define CONNECT_KEY 1234567890  // ����KEY
class C2S_Connect; // Ϊ��VA����������
BUILD_PACKET_BEGIN(C2S_Connect, C2S_PACKET_CONNECT) // �˴���ֹ�ֺ�

// �����Ч����
struct{
    DWORD   m_dwData;

}BUILD_PACKET_END(C2S_Connect)
//����������������������
#pragma pack(pop)   //��
#endif//#pragma once//��
//����������������������