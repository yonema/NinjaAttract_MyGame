#pragma once
#include "StopWatch.h"

namespace nsMyGame
{
	// �O���錾
	namespace nsUI { class CGameMainUI; }	//!< �Q�[�����C����UI�N���X

	/**
	 * @brief �Q�[���X�e�[�g�֘A�̃l�[���X�y�[�X
	*/
	namespace nsGameState
	{
		/**
		 * @brief �Q�[�����C���̃X�e�[�g�N���X
		*/
		class CGameMainState : public IGameObject
		{
		public:		// �R���X�g���N�^�ƃf�X�g���N�^
			/**
			 * @brief �R���X�g���N�^
			*/
			CGameMainState()
			{
				m_instance = this;
			}
			/**
			 * @brief �f�X�g���N�^
			*/
			~CGameMainState()
			{
				m_instance = nullptr;
			}

		public:		// �I�[�o�[���C�h���������o�֐�

			/**
			 * @brief Update�̒��O�ŌĂ΂��J�n����
			 * @return �A�b�v�f�[�g���s�����H
			*/
			bool Start() override final;

			/**
			 * @brief �X�V����
			*/
			void OnDestroy() override final;

			/**
			 * @brief ��������鎞�ɌĂ΂�鏈��
			*/
			void Update() override final;
		public:		// �����o�֐�

			/**
			 * @brief �Q�[���̃^�C���̌v���J�n
			*/
			void StartTimingGame()
			{
				m_isTimeGame = true;	// �^�C�����v��
				m_stopWatch.Start();	// �X�g�b�v�E�H�b�`�̌v���J�n
			}

			/**
			 * @brief �Q�[���̃^�C���̌v���I��
			*/
			void StopTimingGame()
			{
				m_isTimeGame = false;	// �^�C�����v��Ȃ�
				m_stopWatch.Stop();		// �X�g�b�v�E�H�b�`�̌v���I��
			}

			/**
			 * @brief �Q�[���̃^�C���𓾂�
			 * @return 
			*/
			float GetGameTime() const
			{
				return m_gameTimer;
			}

			/**
			 * @brief �S�[������
			*/
			void Goal();

			/**
			 * @brief �S�[���������H�𓾂�
			 * @return �S�[���������H
			*/
			bool IsGoal()const
			{
				return m_isGoal;
			}

		public:		// static�ȃ����o�֐��ƃf�[�^�����o

			static CGameMainState* m_instance;	//!< �C���X�^���X

			/**
			 * @brief �C���X�^���X�𓾂�
			 * @return �C���X�^���X
			*/
			static CGameMainState* GetInstance()
			{
				return m_instance;
			}

		private:	// private�ȃ����o�֐�

			/**
			 * @brief �Q�[���̃^�C�����v������
			*/
			void TimeGame();

		private:	// �f�[�^�����o

			float m_gameTimer = 0.0f;	//!< �Q�[���̃^�C�}�[
			bool m_isTimeGame = false;	//!< �Q�[���̃^�C�����v�邩�H
			nsTimer::CStopWatch m_stopWatch;	//!< �X�g�b�v�E�H�b�`

			nsUI::CGameMainUI* m_gameMainUI = nullptr;	//!< �Q�[�����C����UI�N���X

			bool m_isGoal = false;		//!< �S�[�����Ă��邩�H

		};

		/**
		 * @brief �Q�[�����C���X�e�[�g�̃C���X�^���X�𓾂�
		 * @return �Q�[�����C���X�e�[�g�̃C���X�^���X
		*/
		static CGameMainState* GameMainState()
		{
			return CGameMainState::GetInstance();
		}

	}
}