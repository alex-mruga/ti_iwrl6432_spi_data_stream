import time
import numpy as np


class RadarCube:
    """
    Wrapper for a radar cube stored as a NumPy array with shape
    (n_antenna, n_chirp, n_rangebin).

    Attributes:
        data (np.ndarray):
            Complex‐valued array holding the raw cube, with shape
            (n_antenna, n_chirp, n_rangebin).
        timestamp (float):
            Epoch time in seconds when this frame was captured.
        view (_View):
            Named‐axis accessor for convenient slicing by
            antenna, chirp or rangebin (e.g. cube.view.antenna[0]).

    Usage:
        # create radar cube object
        cube = RadarCube(n_antennas=3, n_chirps=8, n_rangebins=128)

        # access raw data
        raw = cube.data
        print(raw.shape)

        # use view accessor
        ant0       = cube.view.antenna[0]      # Form (8, 128)
        chirp5     = cube.view.chirp[5]        # Form (3, 128)
        rangebin10 = cube.view.rangebin[10]    # Form (3, 8)
    """

    def __init__(self,
                 n_antennas: int,
                 n_chirps: int,
                 n_rangebins: int,
                 timestamp: float = None):
        self.data = np.zeros((n_antennas, n_chirps, n_rangebins), dtype=np.complex64)
        self.timestamp = timestamp or time.time()

        # view object with 3 radar cube axis, so radar cube data can be
        #   accessed more easily via e.g. cubeobj.view.antenna[0]
        self.view = self._View(self.data, dims=("antenna", "chirp", "rangebin"))

    class _View:
        """
        Internal class providing named-axis access to the radar cube array.

        Enables slicing by dimension name (antenna, chirp, rangebin) via dot notation.
        """
        def __init__(self, array: np.ndarray, dims: tuple[str, ...]):
            """
            Args:
                n_antennas (int)    : Number of virtual antennas.
                n_chirps (int)      : Number of Doppler chirps per frame.
                n_rangebins (int)   : Number of range bins per chirp.
                dtype (np.dtype, optional): Data type for array elements.
                    Defaults to np.complex64.
                timestamp (float, optional): Capture time in seconds since epoch.
                    Defaults to current time.
            """
            self._array = array
            self._dims  = dims

            # create an AxisAccessor for each named dimension
            for ax, name in enumerate(dims):
                setattr(self, name, _CubeAxisAccessor(self._array, ax, dims))

        def __repr__(self) -> str:
            """
            Return a string representation of the view, showing shape and dims.
            """
            return f"RadarCube.View(shape={self._array.shape}, dims={self._dims})"


class _CubeAxisAccessor:
    """
    Internal helper enabling axis indexing on the radar cube array.

    Provides a __getitem__ that returns a new view sliced along the specified axis.
    """
    def __init__(self, array: np.ndarray, axis: int, dims: tuple[str, ...]):
        """
        Args:
            array (np.ndarray): The radar cube data array.
            axis (int):         The axis this accessor will operate on (0, 1, or 2).
            dims (tuple[str, ...]): Names of all dimensions.
        """
        self._array = array
        self._axis = axis
        self._dims = dims

    def __getitem__(self, idx: int):
        """
        Slice the array along this accessor's axis and return a new view.

        Args:
            idx (int): Index along the current axis.

        Returns:
            RadarCube._View: A new view object for the sliced data.
        """

        # take a slice along the specified axis without copying data
        sub_array = np.take(self._array, idx, axis=self._axis)

        # build new dimension names excluding the used axis
        new_dims = tuple(d for i, d in enumerate(self._dims) if i != self._axis)

        return RadarCube._View(sub_array, new_dims)

    def __repr__(self) -> str:
        """
        Return a string representation of the CubeAxisAccessor.

        Returns:
            str: Representation including axis index and dimension names.
        """
        return f"<CubeAxisAccessor axis={self._axis} dims={self._dims}>"
