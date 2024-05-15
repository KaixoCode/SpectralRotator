#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Controller.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------
	
	class ControllerData : public Serializable {
	public:

		// ------------------------------------------------

		ControllerData() { init(); }

		// ------------------------------------------------

		void init() override;
		basic_json serialize() override;
		void deserialize(basic_json& data) override;

		// ------------------------------------------------

	};

	// ------------------------------------------------

	class SpectralRotatorController : public Controller {
	public:

		// ------------------------------------------------

		SpectralRotatorController();

		// ------------------------------------------------

	};

	// ------------------------------------------------

}

// ------------------------------------------------
