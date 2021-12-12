#pragma once

namespace nsMyGame
{
	/**
	 * @brief AI�֘A�̃l�[���X�y�[�X
	*/
	namespace nsAI
	{
		/**
		 * @brief �p�X�������N���X
		*/
		class CPath
		{
		private:	// private�ȍ\����
			/**
			 * @brief �p�X�̃Z�N�V����
			*/
			struct SSection {
				Vector3 startPos;	// �Z�N�V�����̊J�n���W�B
				Vector3 endPos;		// �Z�N�V�����̏I�����W�B
				Vector3 direction;	// �Z�N�V�����̕����B
				float length;		// �Z�N�V�����̒����B
			};
		public:		// �R���X�g���N�^�ƃf�X�g���N�^
			/**
			 * @brief �R���X�g���N�^
			*/
			CPath() = default;
			/**
			 * @brief �f�X�g���N�^
			*/
			~CPath() = default;

		public:		// �����o�֐�

			/**
			 * @brief �p�X����ړ�����
			 * @details �������[���h���w�肳��Ă���ƁA�p�X�ړ���ɒn�ʂɃ��C�L���X�g���s���A
			 * ���W��n�ʂɃX�i�b�v���܂��B
			 * @param[in] pos �ړ���������W
			 * @param[in] moveSpeed �ړ����x
			 * @param[out] isEnd �p�X�ړ��I��������true���ݒ肳���
			 * @param[in] physicsWorld �������[���h�B
			 * @return �ړ���̍��W
			*/
			Vector3 Move(
				Vector3 pos,
				const float moveSpeed,
				bool& isEnd,
				const PhysicsWorld* physicsWorld = nullptr
			);

			/**
			 * @brief �N���A
			*/
			void Clear()
			{
				m_sectionNo = 0;
				m_pointArray.clear();
				m_sectionArray.clear();
			}

			/**
			 * @brief �|�C���g�̒ǉ��B
			 * @param[in] point �|�C���g
			*/
			void AddPoint(const Vector3& point)
			{
				m_pointArray.emplace_back(point);
			}

			/**
			 * @brief �p�X���\�z
			*/
			void Build();

		private:	// �f�[�^�����o
			std::vector<Vector3>	m_pointArray;	//!< �|�C���g�̔z��
			std::vector< SSection >	m_sectionArray;	//!< �Z�N�V�����̔z��B
			int m_sectionNo = 0;					//!< �Z�N�V�����ԍ��B
		};

	}
}