#pragma once

namespace mp_server {
	struct pipe_ret_t
	{
		bool success;
		std::string msg;
		pipe_ret_t()
		{
			success = false;
			msg = "";
		}
	};
}
