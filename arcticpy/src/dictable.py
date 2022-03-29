import inspect

from autoconf.class_path import get_class_path, get_class


def as_dict(
        obj
):
    if isinstance(
            obj, list
    ):
        return list(map(
            as_dict,
            obj
        ))
    if obj.__class__.__module__ == 'builtins':
        return obj
    argument_dict = {
        arg: getattr(
            obj, arg
        )
        for arg
        in inspect.getfullargspec(
            obj.__init__
        ).args[1:]
    }
    return {
        "type": get_class_path(
            obj.__class__
        ),
        **{
            key: as_dict(value)
            for key, value
            in argument_dict.items()
        }
    }


class Dictable:
    def dict(self) -> dict:
        """
        A dictionary representation of the instance comprising a type
        field which contains the entire class path by which the type
        can be imported and constructor arguments.
        """
        return as_dict(self)

    @staticmethod
    def from_dict(
            cls_dict
    ):
        """
        Instantiate an instance of a class from its dictionary representation.

        Parameters
        ----------
        cls_dict
            A dictionary representation of the instance comprising a type
            field which contains the entire class path by which the type
            can be imported and constructor arguments.

        Returns
        -------
        An instance of the geometry profile specified by the type field in
        the cls_dict
        """
        if isinstance(
                cls_dict,
                list
        ):
            return list(map(
                Dictable.from_dict,
                cls_dict
            ))
        if not isinstance(
                cls_dict,
                dict
        ):
            return cls_dict

        cls = get_class(
            cls_dict.pop(
                "type"
            )
        )
        # noinspection PyArgumentList
        return cls(
            **{
                name: Dictable.from_dict(
                    value
                )
                for name, value
                in cls_dict.items()
            }
        )

    def __eq__(self, other):
        return self.dict() == other.dict()
