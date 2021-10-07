#pragma once
#include "Noncopyable.h"
#include "AABB.h"
#include "ModelRender.h"


namespace nsMyGame
{

	/**
	 * @brief �����g�����A�N�V�����̃^�[�Q�b�g�֘A�̃l�[���X�y�[�X
	*/
	namespace nsStringActionTarget
	{

		/**
		 * @brief �X�C���O�̃^�[�Q�b�g�֘A�̃l�[���X�y�[�X
		*/
		namespace nsSwingTarget
		{
			/**
			 * @brief �X�C���O�̃^�[�Q�b�g�N���X
			*/
			class CSwingTarget : private nsUtil::Noncopyable
			{
			public:		// �R���X�g���N�^�ƃf�X�g���N�^
				/**
				 * @brief �R���X�g���N�^
				*/
				CSwingTarget() = default;
				/**
				 * @brief �f�X�g���N�^
				*/
				~CSwingTarget();

			public:		// �����o�֐�

				/**
				 * @brief ������
				 * @param[in] modelRender ���f�������_���[
				*/
				void Init(const nsGraphic::nsModel::CModelRender& modelRender);


				/**
				 * @brief �X�C���O�̃^�[�Q�b�g�̍��W���v�Z����
				 * @derails ������ (1, 1, 1) �Ń^�[�Q�b�g�̐���8�i8���_���j
				 * @param[in] widthSegments ���Z�O�����g ( X )
				 * @param[in] heightSegments �����Z�O�����g ( Y )
				 * @param[in] lenghtSegments �����Z�O�����g ( Z )
				*/
				void CalcSwingingTargetPositions(
					const UINT widthSegments,
					const UINT heightSegments,
					const UINT lengthSegments
				);

				/**
				 * @brief �X�C���O�̃^�[�Q�b�g�̍��W�R���e�i���擾
				 * @return �X�C���O�̃^�[�Q�b�g�̍��W�R���e�i
				*/
				const std::vector<Vector3>& GetSwingTargetPositions() const
				{
					return m_swingTargetPositions;
				}

				/**
				 * @brief ���W�𓾂�
				 * @return ���W
				*/
				const Vector3& GetPosition() const
				{
					return m_modelRender->GetPosition();
				}

			private:	// �f�[�^�����o
				nsGeometry::CAABB m_aabb;							//!< AABB�N���X
				std::vector<Vector3> m_swingTargetPositions;		//!< �X�C���O�̃^�[�Q�b�g�̍��W�R���e�i
				const nsGraphic::nsModel::CModelRender* m_modelRender = nullptr;	//!< ���f�������_���[
			};

		}
	}
}


