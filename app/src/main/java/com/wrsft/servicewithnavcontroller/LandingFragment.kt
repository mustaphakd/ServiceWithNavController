package com.wrsft.servicewithnavcontroller

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.databinding.DataBindingUtil
import androidx.fragment.app.activityViewModels
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import androidx.navigation.findNavController
import com.wrsft.servicewithnavcontroller.databinding.FragmentLandingBinding
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import java.util.*

// TODO: Rename parameter arguments, choose names that match
// the fragment initialization parameters, e.g. ARG_ITEM_NUMBER
private const val ARG_PARAM1 = "param1"
private const val ARG_PARAM2 = "param2"

/**
 * A simple [Fragment] subclass.
 * Use the [LandingFragment.newInstance] factory method to
 * create an instance of this fragment.
 */
class LandingFragment : Fragment() {
    // TODO: Rename and change types of parameters
    private var param1: String? = null
    private var param2: String? = null
    private val viewModel: ViewModel by activityViewModels<ViewModel>()

    private lateinit var binding: FragmentLandingBinding
   // private var _binding: FragmentLandingBinding? = null
   //private lateinit var _binding: FragmentLandingBinding; // = null
    // This property is only valid between onCreateView and
// onDestroyView.
   // private val binding get() = _binding!!

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {
            param1 = it.getString(ARG_PARAM1)
            param2 = it.getString(ARG_PARAM2)
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {

        binding = FragmentLandingBinding.inflate(inflater, container, false)
       //var _binding : FragmentLandingBinding = DataBindingUtil.inflate<FragmentLandingBinding>(inflater, R.layout.fragment_landing, container, false)
        val view = binding.root    //binding.root

        binding.log.setOnClickListener { view ->
            val action = LandingFragmentDirections.actionLandingFragmentToLogsFragment()
            view.findNavController().navigate(action)
        }



        disableButtons();

        viewLifecycleOwner.lifecycleScope.launch {
            /*viewModel.canStart.collect{ newValue ->
                binding.start.isClickable = newValue
            }

            viewModel.canStop.collect{ newValue ->
                binding.stop.isClickable = newValue

            } */
            viewLifecycleOwner.repeatOnLifecycle(Lifecycle.State.STARTED) {
                viewModel.runScope()
                viewModel.canStart.collect{newVal ->
                    //view.findViewById<Button>(R.layout.fragment_landing.)
                    binding.start.isEnabled = newVal
                    binding.stop.isEnabled = !newVal
                }
            }


        }
        binding.viewmodel = viewModel;
        return view
    }

    private fun disableButtons() {
        binding.start.isClickable = false
        binding.stop.isClickable = false
    }

    companion object {
        /**
         * Use this factory method to create a new instance of
         * this fragment using the provided parameters.
         *
         * @param param1 Parameter 1.
         * @param param2 Parameter 2.
         * @return A new instance of fragment LandingFragment.
         */
        // TODO: Rename and change types and number of parameters
        @JvmStatic
        fun newInstance(param1: String, param2: String) =
            LandingFragment().apply {
                arguments = Bundle().apply {
                    putString(ARG_PARAM1, param1)
                    putString(ARG_PARAM2, param2)
                }
            }
    }
}