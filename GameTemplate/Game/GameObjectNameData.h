#pragma once

namespace nsNinjaAttract
{
	/**
	 * @brief ���ʃf�[�^
	*/
	namespace nsCommonData
	{

		/**
		 * @brief �Q�[���I�u�W�F�N�g�̖��O�̎��
		*/
		enum EnGameObjectName
		{
			enGN_TitleMap,
			enGN_MainMap,
			enGN_MainGameState,
			enGN_Car,
			enGameObjectNameNum
		};

		/**
		 * @brief �Q�[���I�u�W�F�N�g�̖��O
		*/
		constexpr const char* const kGameObjectName[enGameObjectNameNum]
		{
			"TitleMap",
			"MainMap",
			"MainGameState",
			"Car"
		};
	}
}