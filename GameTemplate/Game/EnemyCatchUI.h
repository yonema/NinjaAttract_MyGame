#pragma once

namespace nsMyGame
{
	// �O���錾
	namespace nsGraphic {
		namespace nsSprite { class CSpriteRender; }	// �X�v���C�g�����_���[
	}
	namespace nsGameState { class CGameMainState; }
	namespace nsAICharacter { class CAICar; }

	/**
	 * @brief UI�֘A�̃l�[���X�y�[�X
	*/
	namespace nsUI
	{
		/**
		 * @brief �G��߂܂��鏈���֘A��UI
		*/
		class EnemyCatchUI : public IGameObject
		{
		public:		// �R���X�g���N�^�ƃf�X�g���N�^
			/**
			 * @brief �R���X�g���N�^
			*/
			EnemyCatchUI() = default;
			/**
			 * @brief �f�X�g���N�^
			*/
			~EnemyCatchUI() = default;

		public:		// �I�[�o�[���C�h���������o�֐�

			/**
			 * @brief Update�̒��O�ŌĂ΂��J�n����
			 * @return �A�b�v�f�[�g���s�����H
			*/
			bool Start() override final;

			/**
			 * @brief ��������鎞�ɌĂ΂�鏈��
			*/
			void OnDestroy() override final;

			/**
			 * @brief �X�V����
			*/
			void Update() override final;

		public:		// �����o�֐�

		private:	// private�ȃ����o�֐�

			/**
			 * @brief �G��߂܂��邱�Ƃ��ł��鍇�}�̃X�v���C�g�̏�����
			*/
			void InitCanCatchEnemeySprite();


		private:	// �f�[�^�����o
			//!< �G��߂܂��邱�Ƃ��ł��鍇�}�̃X�v���C�g�����_���[
			nsGraphic::nsSprite::CSpriteRender* m_canCatchEnemySR = nullptr;

			std::vector<const nsAICharacter::CAICar*> m_aiCarsRef;	//!< �Ԃ�����const�Q��

			nsGameState::CGameMainState* m_gameState = nullptr;	//!< �Q�[���X�e�[�g

		};

	}
}