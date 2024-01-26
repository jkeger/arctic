import numpy as np

class ImagePlus(np.ndarray):

    def __new__(cls, input_array, vv_test=None, covariance=None):
        # Input array is an already formed ndarray instance
        # We first cast to be our class type
        print('in __new__')
        obj = np.asarray(input_array).view(cls)
        # add the new attribute to the created instance
        obj.vv_test = vv_test
        obj.covariance = covariance
        # Finally, we must return the newly created object:
        return obj

    def __array_finalize__(self, obj):
        # see InfoArray.__array_finalize__ for comments
        print('hwww')
        if obj is None: return
        self.vv_test = getattr(obj, 'vv_test', None)
        if hasattr(obj, "covariance"):
            print('hw')
            covariance = obj.covariance
        else: 
            print('else')
            covariance = getattr(obj, 'covariance', None)
            if covariance is None:
                covariance = np.zeros((2,2,5,5))
        self.covariance = covariance

image = np.zeros((4,4))

im=ImPlus(image)

print(im.covariance)
