# Types

b2 will be pretty light on types - it'll have primitive types of:
* `Int8,16,32,64`
* `UInt8,16,32,64`
* `Float32,64`
* `Bool`
* `String`

It has 2 reference types:
* `^T` an owning, move only pointer
* `*T` a non-owning reference, which panics on memory errors

Slice types:
* `[N]T` Stack allocated arrays
* `[]T` Slices
* `[^]T` Dynamic, owning arrays 

User defined types fall into 2 categories:
struct/class/record/product,tuple types:

```
type User = { id: Int32 name: String }
type Admin = { id: Int32 name: String }
type Tuple = {Int32, String}
```

And sum types/tagged unions:
```
type Staff = User 
            | Admin

type Person = user: User
            | admin: Admin
            | citizen
```

Both user defined types have named and unnamed field variants, but if one field is named, they all must be!

sum types have an additional feature when you add names - you can have a variant with no
value at all! This will let us define fun things like:

```
type Maybe(const T: Type) = nothing
                          | just: T

type State = loading
           | running
           | stopped
```

there are definitely issues with this but I'll solve them later!
