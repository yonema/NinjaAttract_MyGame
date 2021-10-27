#include "stdafx.h"
#include "PlayerWallRun.h"
#include "Player.h"
#include "PlayerMovement.h"

namespace nsMyGame
{
	/**
	 * @brief �v���C���[�֘A�̃l�[���X�y�[�X
	*/
	namespace nsPlayer
	{

		/**
		 * @brief �v���C���[�̓����֘A�̃l�[���X�y�[�X
		*/
		namespace nsPlayerMovenent
		{

			/**
			 * @brief ������
			 * @param[in.out] player �v���C���[
			 * @param[in,out] playerMovement �v���C���[�ړ��N���X�̎Q��
			*/
			void CPlayerWallRun::Init(
				CPlayer* player,
				CPlayerMovement* playerMovement
			)
			{
				// �v���C���[�̎Q�Ƃ��Z�b�g
				m_playerRef = player;
				// ���Z�ړ��x�N�g���̎Q�Ƃ��Z�b�g
				m_playerMovementRef = playerMovement;

				return;
			}

			/**
			 * @brief �E�H�[�����������s
			*/
			void CPlayerWallRun::Execute()
			{
				nsDebug::DrawTextPanel(L"[WallRun:Execute]");

				// �d�͂��g�p�s�ɂ���
				m_playerMovementRef->SetUseGravity(false);

				m_playerMovementRef->ResetMoveVecX();
				m_playerMovementRef->ResetMoveVecY();
				m_playerMovementRef->ResetMoveVecZ();

				return;
			}

		}
	}
}